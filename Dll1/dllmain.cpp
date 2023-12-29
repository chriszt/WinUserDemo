#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID reserved)
{
    switch (reason) {
    case DLL_PROCESS_ATTACH: {
        break;
    }
    case DLL_THREAD_ATTACH: {
        break;
    }
    case DLL_THREAD_DETACH: {
        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

