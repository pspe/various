#include <iostream>
#include <vector>

class A
{
public:
  A(void) {}

  void callWork() 
  {
     std::cout << "A:callWork() " << std::endl;
     work();
  }

  void work() 
  { 
    std::cout << "    A:work() " << std::endl; 
  }
};


class B : public A
{
public:
  B(void) {}

  void work() 
  { 
    std::cout << "    B:work() " << std::endl; 
  }
};




template <typename Out>
class C
{
public:
  C(void) {}

  void callWork() 
  {
     std::cout << "A:callWork() " << std::endl;
     work();
  }

  template <typename T>
  Out work(T input) const { return Out(input); }
};






int main()
{
    std::cout << "-------- A" << std::endl; 
    A a;
    a.callWork();
    std::cout << "-------- B" << std::endl; 
    B b;
    b.callWork();
}


