#include "DllTest.h"
#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>

////////////////////////////////////////////////////////////

using AddProc00 = int(__stdcall *)(int, int);

static void ThreadProc00(AddProc00 fnAdd)
{
    //int a = 4, b = 5;
    //int c = fnAdd(a, b);
    //std::fprintf(stdout, "%d + %d = %d\n", a, b, c);
    OutputDebugStringW(L"thread is running");
}

void DllTest00()
{
    std::fprintf(stdout, "pid=%lu\n", GetCurrentProcessId());
    OutputDebugStringW(L"before load Dll1.dll");

    HMODULE mod = LoadLibraryW(L"Dll1.dll");
    if (!mod) {
        std::fprintf(stderr, "load dll failed(%lu)\n", GetLastError());
        return;
    }
    OutputDebugStringW(L"load Dll1.dll succeeded");
    std::fprintf(stdout, "load dll succeeded, mod=0x%p\n", mod);

    AddProc00 fnAdd = (AddProc00)GetProcAddress(mod, "Add");
    if (!fnAdd) {
        std::fprintf(stderr, "get Add address failed(%lu)\n", GetLastError());
        FreeLibrary(mod);
        return;
    }
    std::fprintf(stdout, "get Add address succeeded\n");

    int a = 2, b = 3;
    int c = fnAdd(a, b);
    std::fprintf(stdout, "%d + %d = %d\n", a, b, c);

    OutputDebugStringW(L"before create thread");
    std::thread t(ThreadProc00, fnAdd);
    if (t.joinable()) {
        t.join();
    }
    OutputDebugStringW(L"thread stopped");

    OutputDebugStringW(L"before free Dll1.dll");
    FreeLibrary(mod);
    OutputDebugStringW(L"Dll1.dll unloaded");
}

////////////////////////////////////////////////////////////

static BOOL InjectDll(DWORD pid, WCHAR *dllPath)
{
    BOOL isOK = FALSE;
    HANDLE process = INVALID_HANDLE_VALUE;
    WCHAR *remoteDllPath = NULL;
    HANDLE thread = INVALID_HANDLE_VALUE;
    do {
        process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD |
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
        if (!process) {
            std::fprintf(stderr, "open process failed(%lu)\n", GetLastError());
            break;
        }

        size_t dllPathCch = wcslen(dllPath) + 1;
        size_t dllPathCb = dllPathCch * sizeof(WCHAR);
        remoteDllPath = (WCHAR *)VirtualAllocEx(process, NULL, dllPathCb, MEM_COMMIT, PAGE_READWRITE);
        if (!remoteDllPath) {
            std::fprintf(stderr, "allocate memory from remote process failed(%lu)\n", GetLastError());
            break;
        }

        if (!WriteProcessMemory(process, remoteDllPath, dllPath, dllPathCb, NULL)) {
            std::fprintf(stderr, "write to remote process dll path failed(%lu)\n", GetLastError());
            break;
        }

        LPTHREAD_START_ROUTINE threadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "LoadLibraryW");
        if (!threadProc) {
            std::fprintf(stderr, "get LoadLibraryW address failed(%lu)\n", GetLastError());
            break;
        }

        thread = CreateRemoteThread(process, NULL, 0, threadProc, remoteDllPath, 0, NULL);
        if (!thread) {
            std::fprintf(stderr, "create remote thread failed(%lu)\n", GetLastError());
            break;
        }
        WaitForSingleObject(thread, INFINITE);
        
        isOK = TRUE;
    } while (FALSE);

    if (remoteDllPath) {
        VirtualFreeEx(process, remoteDllPath, 0, MEM_RELEASE);
    }
    if (thread && thread != INVALID_HANDLE_VALUE) {
        CloseHandle(thread);
    }
    if (process && process != INVALID_HANDLE_VALUE) {
        CloseHandle(process);
    }

    return isOK;
}

static BOOL EjectDll(DWORD pid, WCHAR *dllPath)
{
    BOOL isOK = FALSE;
    HANDLE snapShot = INVALID_HANDLE_VALUE;
    HANDLE process = INVALID_HANDLE_VALUE;
    HANDLE thread = INVALID_HANDLE_VALUE;
    do {
        snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (snapShot == INVALID_HANDLE_VALUE) {
            std::fprintf(stderr, "create tool help snap short failed(%lu)\n", GetLastError());
            break;
        }
        
        MODULEENTRY32W me = {sizeof(me)};
        BOOL found = FALSE;
        BOOL moreMods = Module32FirstW(snapShot, &me);
        for (; moreMods; moreMods = Module32NextW(snapShot, &me)) {
            found = (_wcsicmp(me.szModule, dllPath) == 0) || (_wcsicmp(me.szExePath, dllPath) == 0);
            if (found) {
                break;
            }
        }
        if (!found) {
            break;  
        }

        process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD |
            PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pid);
        if (!process) {
            std::fprintf(stderr, "open process failed(%lu)\n", GetLastError());
            break;
        }

        LPTHREAD_START_ROUTINE threadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "FreeLibrary");
        if (!threadProc) {
            std::fprintf(stderr, "get LoadLibraryW address failed(%lu)\n", GetLastError());
            break;
        }
        
        thread = CreateRemoteThread(process, NULL, 0, threadProc, me.modBaseAddr, 0, NULL);
        if (!thread) {
            std::fprintf(stderr, "create remote thread failed(%lu)\n", GetLastError());
            break;
        }
        WaitForSingleObject(thread, INFINITE);

        isOK = TRUE;
    } while (FALSE);

    if (snapShot && snapShot != INVALID_HANDLE_VALUE) {
        CloseHandle(snapShot);
    }
    if (thread && thread != INVALID_HANDLE_VALUE) {
        CloseHandle(thread);
    }
    if (process && process != INVALID_HANDLE_VALUE) {
        CloseHandle(process);
    }

    return isOK;
}

void DllTest01(int argc, char *argv[])
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: DllDemo PID\n");
        return;
    }
    DWORD pid = std::stoul(argv[1]);
    std::fprintf(stdout, "target pid: %lu\n", pid);

    WCHAR dllPath[MAX_PATH];
    GetModuleFileNameW(NULL, dllPath, MAX_PATH);
    WCHAR *fileName = wcsrchr(dllPath, L'\\') + 1;
    wcscpy_s(fileName, MAX_PATH - wcslen(dllPath), L"Dll1.dll");
    std::fprintf(stdout, "dll full path: %ws\n", dllPath);

    Sleep(1000);
    InjectDll(pid, dllPath);
    Sleep(1000);
    EjectDll(pid, dllPath);
}

////////////////////////////////////////////////////////////
