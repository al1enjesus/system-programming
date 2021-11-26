#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#define BUFSIZE 4096 

HANDLE hWriteEvent = NULL;
HANDLE hReadEvent = NULL;
HANDLE hStdin = NULL;

HANDLE hCloseEvent = NULL;

int main(void)
{
    hWriteEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("WriteEvent"));
    hReadEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("ReadEvent"));
    hCloseEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, TEXT("CloseEvent"));
    if (hWriteEvent == NULL || hReadEvent == NULL || hCloseEvent == NULL)
        return -1;

    DWORD MemAddress = NULL;
    BOOL bSuccess;
   
    TCHAR buff[101];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        return -2;

    printf("\n ** This is a message from the processB ** \n");
 
    WaitForSingleObject(hWriteEvent, INFINITE);

    bSuccess = ReadFile(hStdin, &MemAddress, sizeof(DWORD), NULL, NULL);
    if (!bSuccess)
        return -3;
    if (!ReadProcessMemory(GetCurrentProcess(), MemAddress, buff, sizeof(buff), NULL))
        return -4;

    _tprintf(buff);

    printf("\n ** End of processB ** \n");

    if (!SetEvent(hReadEvent))
        return -5;

    WaitForSingleObject(hCloseEvent, INFINITE);

    CloseHandle(hCloseEvent);
    CloseHandle(hWriteEvent);
    CloseHandle(hReadEvent);
    CloseHandle(hStdin);

    return 0;
}