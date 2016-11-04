#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cassert>
#include <memory>


template <typename T>
struct Fncs
{
    static T fncPow (T value) { return std::pow (value, 2); }
    static std::function<T(T)> pFncPow () { return &Fncs<T>::fncPow; }
    

    static int hallo () { return 5; }
};






int main()
{

    std::cout << "return 5 = " << Fncs<double>::hallo () << std::endl;
    auto f = Fncs<double>::pFncPow ();
    std::cout << "f = " << f(4) << std::endl;
}

