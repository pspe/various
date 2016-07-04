
#include "ObjB.hpp"
#include "singleton.hpp"
#include <iostream>

void B::CallSngl()
{
    std::cout << "B::CallSngl" << std::endl;
    Sngl* sngl = Sngl::GetInstance();
    for (size_t i = 0; i < 500; ++i)
	sngl->DoSomething();
}

