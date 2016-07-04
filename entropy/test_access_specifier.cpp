
#include <iostream>

class A
{
public:
private:
    void test () { std::cout << "A/test" << std::endl; }
};


class B : public A
{
public:
    void test () { std::cout << "B/test" << std::endl; }
};



int main ()
{
    std::cout << "A* a = new A" << std::endl;
    A* a = new A;
//    a->test ();

    std::cout << "B* b = new B" << std::endl;
    B* b = new B;
    b->test ();

    std::cout << "A* c = new B" << std::endl;
    A* c = new B;
//    c->test ();
}




