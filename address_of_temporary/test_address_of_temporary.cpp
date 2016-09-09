#include <iostream>

int foo ()
{
    return 2;
}


int main ()
{
    std::cout << "start" << std::endl;
    const int& b = foo ();
    std::cout << "b  = " << b << std::endl;
}
