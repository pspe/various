#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

// template<size_t size> 
// struct overflow{ operator char() { return size + 256; } }; //always overflow
// //if you doubt, you can use UCHAR_MAX +1 instead of 256, to ensure overflow.

template<typename T> 
struct COMPILERPRINT
{ 
    operator char() { return sizeof(T) + 256; }  //always overflow
};
//if you doubt, you can use UCHAR_MAX +1 instead of 256, to ensure overflow.


template <int v>
struct SpecifyInt
{
    enum { value = v };
};

class NullType {};


//template <typename T0, typename T1>
struct Test
{
    template <typename H0, typename H1>
    void operator() (H0& holder0, H1& holder1)
    {
	const int length = 3;
	Initialize (holder0, holder1, SpecifyInt<length>());
    }

private:

    template <typename H0, typename H1, int iPos> 
    void Initialize (H0& holder0, H1& holder1, SpecifyInt<iPos>) 
    {
	return Initialize (holder0, holder1, SpecifyInt<iPos-1>());
    }

    template <typename H0, typename H1> 
    void Initialize (H0& holder0, H1& holder1, SpecifyInt<0>) 
    {
	const int lengthT1 = 2;
	Initialize (holder1, SpecifyInt<lengthT1>());
    }
	
    template <typename H1, int iPos> 
    void Initialize (H1& holder1, SpecifyInt<iPos>) 
    {
	Initialize (holder1, SpecifyInt<iPos-1>());
    }

    template <typename H1> 
    void Initialize (H1& holder1, SpecifyInt<0>) 
    {
	char(COMPILERPRINT<NullType>());
    }
	
};



int main()
{
    Test test;
    int a = 1;
    double b = 2.2;
    test(a,b);
}

