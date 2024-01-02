#include <locale>
#include "ProcessTest.h"

int main()
{
    setlocale(LC_ALL, ".UTF8");

    //ProcessTest00();
    ProcessTest01();
    
    return 0;
}
