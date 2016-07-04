#include <iostream>
#include <vector>


template <typename T>
class Ptr
{
    T* m_t;
public:
    Ptr (T* t) { m_t = t; }
};


template <typename T>
class Base
{
public:
    Base () {}

    template <typename D>
    Ptr<D> Create () 
    {
	return Ptr<D>(new D());
    }
};




template <typename T>
class Derived : public Base<T>
{
public:
    Derived () {}

    Ptr<Derived<T> > Create ()
    {
	return Ptr<Derived<T> >(Base<T>::template Create<Derived<T> >());
	// return Ptr<Derived<T> >(new Derived<T>());
    }
};




int main()
{
    Derived<int> derived;
//    Base<int> base;
    Ptr<Derived<int> > obj = derived.Create();
    // Ptr<Base<int> > obj = base.Create<Base<int> >();
}



