#include "Dll1.h"
#include <Windows.h>

int __stdcall Add(int x, int y)
{
    OutputDebugStringW(L"Dll1!Add");
    return x + y;
}
