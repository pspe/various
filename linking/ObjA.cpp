
#include "ObjA.hpp"
#include "singleton.hpp"
#include <iostream>

void A::CallSngl()
{
    std::cout << "A::CallSngl" << std::endl;
    Sngl* sngl = Sngl::GetInstance();
    for (size_t i = 0; i < 1000; ++i)
	sngl->DoSomething();
}

