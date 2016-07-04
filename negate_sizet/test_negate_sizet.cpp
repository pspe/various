
#include <iostream>
#include <iomanip>

int main ()
{
    unsigned long long a = 0xffffffffffffffff;
    unsigned long long b = (unsigned long long) -1;
    unsigned long long c = (unsigned long long) ~0;


    std::cout << std::hex << a << std::endl;
    std::cout << std::hex << b << std::endl;
    std::cout << std::hex << c << std::endl;
}

