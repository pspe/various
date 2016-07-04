
#include "bitfield.hpp"
#include <iostream>
#include <cassert>



// typedef unsigned int        u_int32;
// typedef signed   int        s_int32;
// //typedef uint64_t            u_int64;
// typedef int64_t             s_int64;

// #define U_INT32_MAX     static_cast<u_int32>(0xFFFFFFFFU)


// inline u_int32 castTo_u_int32(s_int64 v)
// {
//     if (v > U_INT32_MAX || v < 0) *(volatile int*)0 = 13;
//     return (u_int32)v;
// }


int main ()
{

//     s_int64 v = 25;
//     std::cout << v << " --> " << (castTo_u_int32(v)) << std::endl;
//     v = U_INT32_MAX - 1;
//     std::cout << v << " --> " << (castTo_u_int32(v)) << std::endl;
//     v = U_INT32_MAX + 1000;
//     std::cout << v << " --> " << (castTo_u_int32(v)) << std::endl;
//     v = -1;
// //    v = 0xFFFFFFFFFU;
//     std::cout << v << " --> " << (castTo_u_int32(v)) << std::endl;
//     return 0;
    


    std::cout << IKS::BitField (2) << std::endl;
    assert (IKS::BitField (0) != IKS::BitField ());
    assert (IKS::BitField (0) == IKS::BitField (0));
    for (int i = -5, iEnd = 5; i < iEnd; ++i)
    {
        std::cout << "i = " << i << "  bf(i) = " << IKS::BitField (i) << std::endl;
    }


//    unsigned long long value (2);
    unsigned long long value (2);
    for (int shift = 0, shiftEnd = 100; shift < shiftEnd; ++shift)
    {
//        std::cout << "value = " << value << "  >> " << shift << "    shifted value = " << (value >> shift) << std::endl;
        std::cout << "shift = " << shift << "  bf(2,shift) = " << IKS::BitField (2,shift) << std::endl;
    }
    std::cout << std::endl;
    for (int shift = 0, shiftEnd = 100; shift < shiftEnd; ++shift)
    {
//        std::cout << "value = " << value << "  << " << shift << "    shifted value = " << (value << shift) << std::endl;
        std::cout << "shift = " << shift << "  bf(2,shift) = " << IKS::BitField (2,shift) << std::endl;
    }
}


