#include <windows.h>
#include <locale.h>
#include <CommCtrl.h>
#include <strsafe.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <string.h>
#include <stdio.h>

#define ID_LISTBOX 1000
#define ID_WINDEDIT 999

BOOL GetProcessList();
BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);

HWND hList, hInfo;
const char ClassName[] = "MainWindowClass";

typedef struct _ProcessInfo
{
    TCHAR ProcessName[100];
    DWORD ProcessID;
    DWORD ParentProcessID;
    DWORD countThreads;
    DWORD priBase;
    DWORD priClass;
} ProcessInfo;

ProcessInfo* ProcessArr;
int SIZE_PROCESS_ARR = 50, CountProcesses = 0;

MODULEENTRY32* ModuleArr;
int SIZE_MODULE_ARR = 50, CountModules = 0;

THREADENTRY32* ThreadArr;
int SIZE_THREAD_ARR = 50, CountThreads = 0;

int dcount(int d)
{
    if (d == 0)
        return 1;

    int res = 0;
    while (d != 0)
    {
        res++;
        d /= 10;
    }
    return res;
}

void changeBufferSize(TCHAR** buffer, TCHAR** temp_buffer, int* BufferSize, int dif)
{
    int pos = *temp_buffer - *buffer;

    if (pos + dif < *BufferSize)
        return;

    while (pos + dif >= *BufferSize)
        *BufferSize *= 2;

    *buffer = (TCHAR*)realloc(*buffer, *BufferSize * sizeof(TCHAR));
    *temp_buffer = *buffer;
    *temp_buffer += pos;
}

LRESULT CALLBACK winproc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CREATE:
    {
        RECT size;
        GetClientRect(hWnd, &size);
        double width = size.right - size.left;
        double height = size.bottom - size.top;

        hList = CreateWindow(
            L"LISTBOX",
            NULL,
            WS_VISIBLE | WS_CHILD | LBS_NOTIFY | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_LEFT,
            0,
            0,
            width * 1 / 3,
            height,
            hWnd,
            (HMENU)ID_LISTBOX,
            (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
            NULL);

        GetProcessList();
        for (int i = 0; i < CountProcesses; i++)
        {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ProcessArr[i].ProcessName);
        }

        RECT rect;
        GetClientRect(hList, &rect);

        hInfo = CreateWindow(
            WC_EDITW,
            NULL,
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL
            | ES_LEFT | ES_AUTOHSCROLL,
            rect.right,
            rect.top,
            width * 2 / 3,
            height,
            hWnd,
            (HMENU)ID_WINDEDIT,
            (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE),
            NULL);
        break;
    }
    case WM_SIZE:
    {
        double width = LOWORD(lParam);
        double height = HIWORD(lParam);
        switch (wParam)
        {
        case SIZE_MAXIMIZED:
        {
            SetWindowPos(hList, HWND_TOP, 0, 0, width * 1 / 3, height, SWP_ASYNCWINDOWPOS);
            SetWindowPos(hInfo, HWND_TOP, width * 1 / 3, 0, width * 2 / 3, height, SWP_ASYNCWINDOWPOS);
            break;
        }
        case SIZE_RESTORED:
        {
            SetWindowPos(hList, HWND_TOP, 0, 0, width * 1 / 3, height, SWP_ASYNCWINDOWPOS);
            SetWindowPos(hInfo, HWND_TOP, width * 1 / 3, 0, width * 2 / 3, height, SWP_ASYNCWINDOWPOS);
            break;
        }
        }
        break;
    }
    case WM_SIZING:
    {
        RECT size, rect;
        GetClientRect(hWnd, &size);
        double width = size.right - size.left;
        double height = size.bottom - size.top;

        GetClientRect(hList, &rect);
        SetWindowPos(hList, HWND_TOP, 0, 0, width * 1 / 3, height, SWP_ASYNCWINDOWPOS);
        SetWindowPos(hInfo, HWND_TOP, rect.right, 0, width * 2 / 3, height, SWP_ASYNCWINDOWPOS);
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_LISTBOX:
        {
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
            {
                HWND hwndList = GetDlgItem(hWnd, ID_LISTBOX);

                int index = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                int BufferSize = 1000;
                TCHAR* buff = malloc(sizeof(TCHAR) * BufferSize);
                TCHAR* temp = buff;

                changeBufferSize(&buff, &temp, &BufferSize, 17 + dcount(ProcessArr[index].ProcessID));
                StringCbPrintf(temp, (17 + dcount(ProcessArr[index].ProcessID)) * sizeof(TCHAR),
                    TEXT("Process ID:     %d"), ProcessArr[index].ProcessID);
                temp += 16 + dcount(ProcessArr[index].ProcessID);

                changeBufferSize(&buff, &temp, &BufferSize, 19 + dcount(ProcessArr[index].ParentProcessID));
                StringCbPrintf(temp, (19 + dcount(ProcessArr[index].ParentProcessID)) * sizeof(TCHAR),
                    TEXT("\r\nParent ID:      %d"), ProcessArr[index].ParentProcessID);
                temp += 18 + dcount(ProcessArr[index].ParentProcessID);

                changeBufferSize(&buff, &temp, &BufferSize, 19 + 4);
                StringCbPrintf(temp, (19 + 4) * sizeof(TCHAR),
                    TEXT("\r\nThread count:       "));
                temp += 18 + 4;

                changeBufferSize(&buff, &temp, &BufferSize, 19 + dcount(ProcessArr[index].priBase));
                StringCbPrintf(temp, (19 + dcount(ProcessArr[index].priBase)) * sizeof(TCHAR),
                    TEXT("\r\nPriority base:  %d"), ProcessArr[index].priBase);
                temp += 18 + dcount(ProcessArr[index].priBase);

                if (ProcessArr[index].priClass != 0)
                {
                    changeBufferSize(&buff, &temp, &BufferSize, 19 + dcount(ProcessArr[index].priClass));
                    StringCbPrintf(temp, (19 + dcount(ProcessArr[index].priClass)) * sizeof(TCHAR),
                        TEXT("\r\nPriority class: %d"), ProcessArr[index].priClass);
                    temp += 18 + dcount(ProcessArr[index].priClass);
                }

                if (ListProcessModules(ProcessArr[index].ProcessID))
                {
                    for (int i = 0; i < CountModules; i++)
                    {
                        changeBufferSize(&buff, &temp, &BufferSize, 27 + lstrlen(ModuleArr[i].szModule));
                        StringCbPrintf(temp, (27 + lstrlen(ModuleArr[i].szModule)) * sizeof(TCHAR),
                            TEXT("\r\n\r\n     MODULE NAME:     %s"), ModuleArr[i].szModule);
                        temp += 26 + lstrlen(ModuleArr[i].szModule);

                        changeBufferSize(&buff, &temp, &BufferSize, 25 + lstrlen(ModuleArr[i].szExePath));
                        StringCbPrintf(temp, (25 + lstrlen(ModuleArr[i].szExePath)) * sizeof(TCHAR),
                            TEXT("\r\n     Executable     = %s"), ModuleArr[i].szExePath);
                        temp += 24 + lstrlen(ModuleArr[i].szExePath);

                        changeBufferSize(&buff, &temp, &BufferSize, 25 + dcount(ModuleArr[i].th32ProcessID));
                        StringCbPrintf(temp, (25 + dcount(ModuleArr[i].th32ProcessID)) * sizeof(TCHAR),
                            TEXT("\r\n     Process ID     = %d"), ModuleArr[i].th32ProcessID);
                        temp += 24 + dcount(ModuleArr[i].th32ProcessID);

                        changeBufferSize(&buff, &temp, &BufferSize, 25 + dcount(ModuleArr[i].modBaseSize));
                        StringCbPrintf(temp, (25 + dcount(ModuleArr[i].modBaseSize)) * sizeof(TCHAR),
                            TEXT("\r\n     Base size      = %d"), ModuleArr[i].modBaseSize);
                        temp += 24 + dcount(ModuleArr[i].modBaseSize);
                    }
                }
                if (ListProcessThreads(ProcessArr[index].ProcessID))
                {
                    for (int i = 0; i < CountThreads; i++)
                    {
                        changeBufferSize(&buff, &temp, &BufferSize, 27 + dcount(ThreadArr[i].th32ThreadID));
                        StringCbPrintf(temp, (27 + dcount(ThreadArr[i].th32ThreadID)) * sizeof(TCHAR),
                            TEXT("\r\n\r\n     THREAD ID      = %d"), ThreadArr[i].th32ThreadID);
                        temp += 26 + dcount(ThreadArr[i].th32ThreadID);

                        changeBufferSize(&buff, &temp, &BufferSize, 25 + dcount(ThreadArr[i].tpBasePri));
                        StringCbPrintf(temp, (25 + dcount(ThreadArr[i].tpBasePri)) * sizeof(TCHAR),
                            TEXT("\r\n     Base priority  = %d"), ThreadArr[i].tpBasePri);
                        temp += 24 + dcount(ThreadArr[i].tpBasePri);
                    }
                }

                ProcessArr[index].countThreads = CountThreads;
                temp = buff;
                temp += 16 + dcount(ProcessArr[index].ProcessID) + 18 + dcount(ProcessArr[index].ParentProcessID) + 18;
                StringCbPrintf(temp, (1+dcount(CountThreads)) * sizeof(TCHAR),
                    TEXT("%d"), CountThreads);
                temp += dcount(CountThreads);
                *temp = TEXT(' ');

                SetWindowText(hInfo, buff);
                free(buff);
                free(ModuleArr);
                free(ThreadArr);
                CountModules = 0;
                CountThreads = 0;
            }
            }
        }
        break;
        }
        return 0;
    }
    break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return (DefWindowProc(hWnd, Msg, wParam, lParam));
    }
    return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    setlocale(LC_ALL, "");

    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = winproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = ClassName;

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Failed To Register The Window Class.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND    hWnd;
    hWnd = CreateWindowEx(
        WS_EX_LEFT,
        ClassName,
        L"Task Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    MSG    Msg;

    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

BOOL GetProcessList()
{
    ProcessArr = (ProcessInfo*)malloc(sizeof(ProcessInfo) * SIZE_PROCESS_ARR);

    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return(FALSE);
    }

    int i = 0;
    do
    {
        if (i == SIZE_PROCESS_ARR)
        {
            SIZE_PROCESS_ARR *= 2;
            ProcessArr = (ProcessInfo*)realloc(ProcessArr, SIZE_PROCESS_ARR * sizeof(ProcessInfo));
        }

        dwPriorityClass = 0;

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL)
        {
            dwPriorityClass = GetPriorityClass(hProcess);
            if (!dwPriorityClass)
            {
                CloseHandle(hProcess);
            }
        }

        lstrcpy((*(ProcessArr + i)).ProcessName, pe32.szExeFile);
        (*(ProcessArr + i)).ProcessID = pe32.th32ProcessID;
        (*(ProcessArr + i)).ParentProcessID = pe32.th32ParentProcessID;
        (*(ProcessArr + i)).countThreads = pe32.cntThreads;
        (*(ProcessArr + i)).priBase = pe32.pcPriClassBase;
        (*(ProcessArr + i)).priClass = dwPriorityClass;

        i++;

    } while (Process32Next(hProcessSnap, &pe32));

    CountProcesses = i;
    CloseHandle(hProcessSnap);
    return(TRUE);
}

BOOL ListProcessModules(DWORD dwPID)
{
    ModuleArr = (MODULEENTRY32*)malloc(sizeof(MODULEENTRY32) * SIZE_MODULE_ARR);

    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    me32.dwSize = sizeof(MODULEENTRY32);

    if (!Module32First(hModuleSnap, &me32))
    {
        CloseHandle(hModuleSnap);
        return(FALSE);
    }

    int i = 0;
    do
    {
        if (i == SIZE_MODULE_ARR)
        {
            SIZE_MODULE_ARR *= 2;
            ModuleArr = (MODULEENTRY32*)realloc(ModuleArr, sizeof(MODULEENTRY32) * SIZE_MODULE_ARR);
        }

        *(ModuleArr + i) = me32;

        i++;
    } while (Module32Next(hModuleSnap, &me32));

    CountModules = i;
    CloseHandle(hModuleSnap);
    return(TRUE);
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
    ThreadArr = (THREADENTRY32*)malloc(sizeof(THREADENTRY32) * SIZE_THREAD_ARR);

    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32))
    {
        CloseHandle(hThreadSnap);
        return(FALSE);
    }

    int i = 0;
    do
    {
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            if (i == SIZE_THREAD_ARR)
            {
                SIZE_THREAD_ARR *= 2;
                ThreadArr = (THREADENTRY32*)realloc(ThreadArr, sizeof(THREADENTRY32) * SIZE_THREAD_ARR);
            }

            ThreadArr[i] = te32;

            i++;
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CountThreads = i;
    CloseHandle(hThreadSnap);
    return(TRUE);
}