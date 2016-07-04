#include "bitfield.hpp"
#include <iomanip>


typedef unsigned int        u_int32;
typedef signed   int        s_int32;
//typedef uint64_t            u_int64;
typedef int64_t             s_int64;

#define U_INT32_MAX     static_cast<u_int32>(0xFFFFFFFFU)


inline u_int32 castTo_u_int32(s_int64 v)
{
    if (v > U_INT32_MAX || v < 0) *(volatile int*)0 = 13;
    return (u_int32)v;
}


std::ostream& operator<<(std::ostream& out, const IKS::BitField& bitField) 
{

    // u_int64 test5 = u_int64(ONES64) << 5;
    // u_int64 test15 = u_int64(ONES64) << 15;
    // u_int64 test25 = u_int64(ONES64) << 25;
    // u_int64 test35 = u_int64(ONES64) << 35;
    // u_int64 test45 = u_int64(ONES64) << 45;
    // u_int64 test55 = u_int64(ONES64) << 55;
    // u_int64 test63 = u_int64(ONES64) << 63;
    // u_int64 test64 = u_int64(ONES64) << 64;
    // u_int64 test65 = u_int64(ONES64) << 65;

    s_int64 v = 25;
    out << (castTo_u_int32(v)) << "v";
    return out;
    
	out << "#blocks = " << bitField.m_blocks.size() << " | ext = " << bitField.m_extensionBit << " | ";
	std::vector<u_int64>::const_iterator it = bitField.m_blocks.begin(), itEnd = bitField.m_blocks.end();
	while (it != itEnd)
	{
		--itEnd;
		out << std::hex << std::setfill('0') << std::setw(16) << (*itEnd) << " ";
	}
	return out;
}



