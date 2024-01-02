#include "ProcessTest.h"
#include <iostream>
#include <Windows.h>

////////////////////////////////////////////////////////////

void ProcessTest00()
{
    std::fprintf(stdout, "pid=%lu\n", GetCurrentProcessId());
    int count = 0;
    while (true) {
        std::cout << "count: " << count++ << std::endl;
        Sleep(1000);
    }
}

////////////////////////////////////////////////////////////

extern "C" const IMAGE_DOS_HEADER __ImageBase;

void ProcessTest01()
{
    HMODULE mod1 = GetModuleHandleW(NULL);
    HINSTANCE inst1 = (HINSTANCE)&__ImageBase;
    HMODULE mod2 = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)ProcessTest01, &mod2);
    std::fprintf(stdout, "mod1=0x%p, inst1=0x%p, mod2=0x%p\n", mod1, inst1, mod2);
}

////////////////////////////////////////////////////////////
