#include <Windows.h>

#define BUFFER_SIZE 1024

static void PrintContext(HMODULE mod, DWORD reason, LPVOID reserved)
{
    WCHAR buf[BUFFER_SIZE];
    wsprintfW(buf, L"mod=0x%p, reason=%lu, reserved=0x%p", mod, reason, reserved);
    OutputDebugStringW(buf);
}

static void WalkDlls(HMODULE mod)
{
    char buf[MAX_PATH * 100]{0};
    PBYTE pb = NULL;
    MEMORY_BASIC_INFORMATION mbi;
    while (VirtualQuery(pb, &mbi, sizeof(MEMORY_BASIC_INFORMATION)) == sizeof(MEMORY_BASIC_INFORMATION)) {
        int len;
        char modName[MAX_PATH];
        if (mbi.State == MEM_FREE) {
            mbi.AllocationBase = mbi.BaseAddress;
        }
        if ((mbi.AllocationBase == mod) || (mbi.AllocationBase != mbi.BaseAddress) || (mbi.AllocationBase == NULL)) {
            len = 0;
        } else {
            len = GetModuleFileNameA((HMODULE)mbi.AllocationBase, modName, MAX_PATH);
        }
        if (len > 0) {
            wsprintfA(strchr(buf, 0), "\n0x%p-%s", mbi.AllocationBase, modName);
        }
        pb += mbi.RegionSize;
    }
    OutputDebugStringA(buf);
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID reserved)
{
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            PrintContext(mod, reason, reserved);
            WalkDlls(mod);
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

