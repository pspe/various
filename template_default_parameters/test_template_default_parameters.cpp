#include <iostream>


template <typename T0, typename T1 = T0>
struct A
{
    T0 getSomeT0 () { return T0(1.3); }
    T1 getSomeT1 () { return T1(1.3); }
};



int main ()
{
    A<int> a_int;
    A<int,int> a_int_int;
    A<int,double> a_int_double;
    A<double,int> a_double_int;

    std::cout << "a_int: T0: " << a_int.getSomeT0() << "  T1: " << a_int.getSomeT1() << std::endl;
    std::cout << "a_int_int: T0: " << a_int_int.getSomeT0() << "  T1: " << a_int_int.getSomeT1() << std::endl;
    std::cout << "a_int_double: T0: " << a_int_double.getSomeT0() << "  T1: " << a_int_double.getSomeT1() << std::endl;
    std::cout << "a_double_int: T0: " << a_double_int.getSomeT0() << "  T1: " << a_double_int.getSomeT1() << std::endl;
}
