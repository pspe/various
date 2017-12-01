#include <iostream>
//#include <iomanip>
#include <vector>
//#include <algorithm>
//#include <iterator>
//#include <functional>
//#include <set>
//#include <chrono>
#include <cassert>


#include "smartptr.h"

//using namespace std::chrono;




class Test : public IKS::IRefCounter
{
public:
    Test (int _a, int _b)
	: a (_a)
	, b (_b)
    {}
    
    Test (const Test& other)
	: a (other.a)
	, b (other.b)
    {}

    void print () const
    {
	std::cout << "a = " << a << " b = " << b << std::endl;
    }

    Test& operator+= (const Test& other)
    {
	a += other.a;
	b += other.b;
	return *this;
    }
    
private:
    ~Test () {}

private:
    int a;
    int b;
};




template <typename T>
void mySwap (T& a, T& b)
{
    T tmp = std::move (a);
    a = std::move (b);
    b = std::move (tmp);
}



int main()
{
    IKS::Ptr<Test> A (new Test (1,3));
    IKS::Ptr<Test> B (new Test (2,4));

    *A += *B;
    A->print ();
    B->print ();

    // std::swap (A, B);
    mySwap (A, B);
    A->print ();
    B->print ();
    
    return 0;
}

