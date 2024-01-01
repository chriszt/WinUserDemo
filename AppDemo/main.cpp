#include <iostream>
#include <Windows.h>

int main()
{
    std::fprintf(stdout, "pid=%lu\n", GetCurrentProcessId());
    int count = 0;
    while (true) {
        std::cout << "count: " << count++ << std::endl;
        Sleep(1000);
    }
    return 0;
}
