#ifndef __OBJ_B_HPP__
#define __OBJ_B_HPP__

#define DLL_PUBLIC __attribute__ ((visibility ("default")))


class DLL_PUBLIC B
{
public:
    B(void) {}
    
    void CallSngl();
};

#endif

