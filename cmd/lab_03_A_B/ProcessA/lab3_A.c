#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 4096

HANDLE hReadPipe = NULL;
HANDLE hWritePipe = NULL;

HANDLE hWriteEvent = NULL;
HANDLE hReadEvent = NULL;

HANDLE hCloseEvent = NULL;

HANDLE CreateChildProcess(void);
void WriteToPipe(DWORD);

int main(void)
{
    hWriteEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("WriteEvent"));
    hReadEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("ReadEvent"));
    hCloseEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("CloseEvent"));

    if (hWriteEvent == NULL || hReadEvent == NULL || hCloseEvent == NULL)
        return 1;

    printf("\n->Start of parent execution.\n\n");

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
        return 2;

    printf("Enter a message: ");

    TCHAR buff[100];
    _tscanf_s(TEXT("%99[^\n]"), buff, (unsigned)_countof(buff));

    HANDLE hProcess = CreateChildProcess();
    DWORD pAddress = VirtualAllocEx(hProcess, NULL, sizeof(buff), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (hProcess == NULL)
        return 3;
    if (pAddress == NULL)
        return 4;

    if (!WriteProcessMemory(hProcess, pAddress, buff, sizeof(buff), NULL))
        return 5;

    printf("\n->Writing to pipe\n");
    WriteToPipe(pAddress);

    WaitForSingleObject(hReadEvent, INFINITE);
    printf("\n->Cleaning the allocated memory\n");
   
    if (!VirtualFreeEx(hProcess, pAddress, 0, MEM_RELEASE))
        return 6;
    
    printf("\n->End of processA\n");

    printf("\n->Press Enter to end the processB\n");
    
    while ((getchar()) != '\n');
    int c = getchar();
    if (!SetEvent(hCloseEvent))
        return 10;

    CloseHandle(hProcess);
    CloseHandle(hWriteEvent);
    CloseHandle(hReadEvent);
    CloseHandle(hReadPipe);
    CloseHandle(hWritePipe);
    CloseHandle(hCloseEvent);

    return 0;
}

HANDLE CreateChildProcess()
{
    TCHAR szCmdline[] = TEXT("ProcessB");
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdInput = hReadPipe;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(NULL,
        szCmdline,          // command line 
        NULL,               // process security attributes 
        NULL,               // primary thread security attributes 
        TRUE,               // handles are inherited 
        CREATE_NEW_CONSOLE, // creation flags 
        NULL,               // use parent's environment 
        NULL,               // use parent's current directory 
        &siStartInfo,       // STARTUPINFO pointer 
        &piProcInfo);       // receives PROCESS_INFORMATION 
 
    if (!bSuccess)
        return 7;

    CloseHandle(piProcInfo.hThread);

    return piProcInfo.hProcess;
}

void WriteToPipe(DWORD MemAddress)
{
    BOOL bSuccess = FALSE;

    bSuccess = WriteFile(hWritePipe, &MemAddress, sizeof(DWORD), NULL, NULL);
    if (!bSuccess)
        return 8;

    if (!SetEvent(hWriteEvent))
        return 9;
}