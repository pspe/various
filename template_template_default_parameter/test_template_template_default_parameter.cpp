#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>



template <typename T>
class X
{
public:
    static T value()
    {
	return T(3.3);
    }
};

template <typename T>
class Y
{
public:
    static T value()
    {
	return T(8.8);
    }
};


template <typename X, template <typename A> class T, template <typename B> class U = T>
class TempTempTest
{
public:
    int action()
    {
	return T<int>::value()+U<int>::value();
    }
};










int main()
{


    TempTempTest<Y<int>,X,X> ttt0;
    std::cout << "TTT0: " << ttt0.action() << std::endl;
    TempTempTest<Y<int>,X> ttt1;
    std::cout << "TTT1: " << ttt1.action() << std::endl;
    TempTempTest<int,X> ttt2;
    std::cout << "TTT2: " << ttt2.action() << std::endl;
 

}



