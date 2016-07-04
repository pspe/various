#include <iostream>
#include <string>




template <typename T>
class Iterator
{
public:
    Iterator() {}
    virtual std::string HasNext () {return "interface";}
    
};


class Interface
{
public:
    typedef Iterator<Interface> iterator;
    virtual iterator getIterator () = 0;
    virtual bool doSomething() = 0;
};





class A : public Interface
{
public:
//    typedef Iterator<A> iterator;
    virtual iterator getIterator ();


    virtual bool doSomething() {return true;}
};



class B : public Interface
{
public:
    virtual iterator getIterator ();


    virtual bool doSomething() {return false;}
};





template <>
class Iterator<A> : public Iterator<Interface>
{
public:
    std::string HasNext () {return "iterator A";}
    
};


template <>
class Iterator<B> : public Iterator<Interface>
{
public:
    std::string HasNext () {return "iterator B";}
    
};



Interface::iterator A::getIterator () 
{ 
    return Iterator<A>(); 
}


Interface::iterator B::getIterator () 
{ 
    return Iterator<B>(); 
}


int main ()
{
    A a;
    Interface::iterator it = a.getIterator();
    std::cout << "iterator: " << it.HasNext() << std::endl;

    B b;
    Interface::iterator itB = b.getIterator();
    std::cout << "iterator: " << itB.HasNext() << std::endl;
}
