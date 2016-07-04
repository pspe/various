#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__


class Sngl
{
public:
    Sngl(){}
    static Sngl* GetInstance();
    void         DoSomething();

private:
    static Sngl* m_pInstance;
};


#endif

