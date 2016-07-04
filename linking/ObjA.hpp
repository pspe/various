#ifndef __OBJ_A_HPP__
#define __OBJ_A_HPP__

#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
//#define DLL_PUBLIC


class DLL_PUBLIC A
{
public:
    A(void) {}
    
    /*DLL_LOCAL*/ void CallSngl();
};

#endif

