#include "singleton.hpp"
#include <iostream>

Sngl* Sngl::m_pInstance = 0;

Sngl* Sngl::GetInstance()
{
    std::cout << "Sngl::GetInstance()" << std::endl;
    if (m_pInstance == 0)
    {
	std::cout << "Sngl::GetInstance()/new Sngl" << std::endl;
	m_pInstance = new Sngl;
    }
    return m_pInstance;
}

void Sngl::DoSomething()
{
    static size_t count = 0;
    std::cout << "DoSomething -- " << this << "  " << (count++) << std::endl;
}

