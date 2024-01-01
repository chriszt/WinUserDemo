#include <Windows.h>

#define BUFFER_SIZE 1024

static void PrintContext(HMODULE mod, DWORD reason, LPVOID reserved)
{
    WCHAR buf[BUFFER_SIZE];
    wsprintfW(buf, L"mod=0x%p, reason=%lu, reserved=0x%p", mod, reason, reserved);
    OutputDebugStringW(buf);
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID reserved)
{
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            PrintContext(mod, reason, reserved);
            break;
        }
        case DLL_THREAD_ATTACH: {
            PrintContext(mod, reason, reserved);
            break;
        }
        case DLL_THREAD_DETACH: {
            PrintContext(mod, reason, reserved);
            break;
        }
        case DLL_PROCESS_DETACH:
            PrintContext(mod, reason, reserved);
            break;
        }
    return TRUE;
}

