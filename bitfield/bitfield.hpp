#ifndef __BITFIELD_HPP__
#define __BITFIELD_HPP__

#include <vector>
#include <iostream>


#define UI64 UL
//#define CU64(a)    (a##UI64)
#define CU64(a)    (a##UL)
#define ONES64 CU64(0xffffffffffffffff)


//typedef unsigned __int64 u_int64;
typedef unsigned long long u_int64;



namespace IKS
{
    class BitField;
}
std::ostream& operator<<(std::ostream& out, const IKS::BitField& bitField);





namespace IKS
{


    class BitField
    {
    public:
        //BitField(int size, bool flagsSet)
        //{
        //    if (flagsSet)
        //        m_baseBitField = CU64(0xffffffffffffffff);
        //    else
        //        m_baseBitField = 0;

        //    for( int i = 64; i < size; i += 64)
        //    {
        //        if (flagsSet)
        //            m_additionalBitFields.push_back(CU64(0xffffffffffffffff));
        //        else
        //            m_additionalBitFields.push_back(0);
        //    }
        //}

	template <typename T>
	BitField(T value, size_t shift = 0);

	BitField(const BitField& value, size_t shift = 0);
	BitField(bool value, size_t shift = 0);



	BitField() : m_extensionBit (false) {}



        //inline bool checkSingleFlag(int index) const;
        //inline bool checkMatch(const BitField other) const;
        //inline bool empty() const;
        //inline bool full() const;

        //inline void mergeAnd(const BitField other);
        //inline void mergeOr(const BitField other);
        //inline void setSingleFlag(int index);
        //inline void addSingleFlag(int index);

	// *************** setters *******************************
	template <typename T>
	inline BitField& set   (T value, size_t shift);
	template <typename T>
	inline BitField& clear (T value, size_t shift);

	// *************** getters *******************************
	template <typename T>
	T get (size_t shiftFrom=0, size_t size=0) const;
	

	// *************** sub field handling ********************
	inline BitField  subField (size_t start, size_t size);
	inline BitField& set      (const BitField& bitField, size_t start);



	// *************** arithmetic operations *****************
	inline BitField  operator>> (size_t shift) const;
	inline BitField  operator<< (size_t shift) const;

	inline BitField& operator>>= (size_t shift);
	inline BitField& operator<<= (size_t shift);
	
	inline BitField  operator|  (const BitField& other) const;
	inline BitField& operator|= (const BitField& other);

	inline BitField  operator&  (const BitField& other) const;
	inline BitField& operator&= (const BitField& other);

	inline BitField  operator~  () const;

	
	// *************** comparison operators *******************
	bool operator==  (const BitField& other) const;
	bool operator<   (const BitField& other) const;
	bool operator<=  (const BitField& other) const
	{
            return (*this == other) || (*this < other);
	}
	bool operator>   (const BitField& other) const
	{
            return !(*this <= other);
	}
	bool operator>=  (const BitField& other) const
	{
            return !(*this < other);
	}
	bool operator!=  (const BitField& other) const
	{
            return !(*this == other);
	}


	bool empty () const { return !m_extensionBit && m_blocks.empty (); }
	bool full  () const { return m_extensionBit && m_blocks.empty (); }

	size_t blocks () const { return m_blocks.size (); }

    private:

	BitField(double value, size_t shift = 0); // setting double not allowed 
	BitField(float  value, size_t shift = 0); // setting float not allowed 

	inline BitField& set   (double value, size_t shift);
	inline BitField& set   (float  value, size_t shift);

	inline BitField& clear (double value, size_t shift);
	inline BitField& clear (float  value, size_t shift);

    private:
        std::vector<u_int64> m_blocks;
	bool m_extensionBit;


	friend std::ostream& ::operator<<(std::ostream& out, const BitField& bitField);
    };



    inline bool BitField::operator== (const BitField& other) const
    {
	if (m_extensionBit != other.m_extensionBit)
            return false;

	if (m_blocks.size () != other.m_blocks.size ())
            return false;

	for (std::vector<u_int64>::const_iterator it = m_blocks.begin(), itOther = other.m_blocks.begin(), 
                 itEnd = m_blocks.end()/*, itOtherEnd = other.m_blocks.end()*/; it != itEnd; ++it, ++itOther)
	{
            if (*it != *itOther)
                return false;
	}
	return true;
    }


    inline bool BitField::operator<  (const BitField& other) const
    {
	if (m_extensionBit)
	{
            if (other.m_extensionBit)
            {
                if (m_blocks.size() > other.m_blocks.size ())
                {
                    return true;
                }
                else if (m_blocks.size() < other.m_blocks.size ())
                {
                    return false;
                }
                else
                {
                    if (m_blocks.empty ())
                        return false;

                    std::vector<u_int64>::const_iterator it = m_blocks.end (), itStart = m_blocks.begin ();
                    std::vector<u_int64>::const_iterator itOther = other.m_blocks.end ()/*, itOtherStart = other.m_blocks.begin ()*/;
                    do
                    {
                        --it;
                        --itOther;
                        if (*it < *itOther)
                            return true;
                    }
                    while (it != itStart);
                }
            }
            else
            {
                return true;
            }
	}
	else
	{
            if (other.m_extensionBit)
            {
                return false;
            }
            else
            {
                if (m_blocks.size() > other.m_blocks.size ())
                {
                    return false;
                }
                else if (m_blocks.size() < other.m_blocks.size ())
                {
                    return true;
                }
                else
                {
                    if (m_blocks.empty ())
                        return false;

                    std::vector<u_int64>::const_iterator it = m_blocks.end (), itStart = m_blocks.begin ();
                    std::vector<u_int64>::const_iterator itOther = other.m_blocks.end ()/*, itOtherStart = other.m_blocks.begin ()*/;
                    do
                    {
                        --it;
                        --itOther;
                        if (*it < *itOther)
                            return true;
                        else if (*it > *itOther)
                            return false;
                    }
                    while (it != itStart);
                }
            }
	}
	return false;
    }

    inline BitField::BitField(const IKS::BitField& value, size_t shift) 
    {
	m_extensionBit = value.m_extensionBit;
	for (std::vector<u_int64>::const_iterator it = value.m_blocks.begin(), itEnd = value.m_blocks.end(); it != itEnd; ++it)
            m_blocks.push_back (*it);
	*this <<= shift;
    }


    inline BitField::BitField(bool value, size_t shift) : m_extensionBit(!value)
    {
//	if (shift == 0)
//            return;

	if (value)
	{
            while (shift > 64)
            {
                m_blocks.push_back (ONES64);
                shift -= 64;
            }
            unsigned short complementaryShift = 64-shift;
            u_int64 filler = complementaryShift >= 64 ? 0 : ONES64 >> complementaryShift;
            m_blocks.push_back (filler);
	}
	else
	{
            while (shift > 64)
            {
                m_blocks.push_back (0);
                shift -= 64;
            }
            u_int64 filler = shift >= 64 ? 0 : ONES64 << shift;
            m_blocks.push_back (filler);

	}
    }


    template <typename T>
    inline BitField::BitField(T value, size_t shift) : m_extensionBit(false)
    {
//	if (value == 0)
//		return;

	if (value < 0)
	{
            m_extensionBit = true;
	}

	m_blocks.push_back (value);
	*this <<= shift;
    }


// *************** setters *******************************
    template <typename T>
    inline BitField& BitField::set   (T value, size_t shift)
    {
	BitField bitField (value, shift);
	return *this |= bitField;
    }

    inline BitField& BitField::set      (const BitField& bitField, size_t shift)
    {
	BitField bf (bitField);
	bf <<= shift;
	return *this |= bf;
    }



    template <typename T>
    inline BitField& BitField::clear (T value, size_t shift)
    {
	BitField bitField (value, shift);
	return *this &= ~bitField;
    }


    inline BitField  BitField::subField (size_t start, size_t size)
    {
	BitField bf (*this);
	bf >>= start;
	BitField mask (true, size);
	return bf |= mask;
    }



    template <typename T>
    T BitField::get (size_t shiftFrom, size_t size) const
    {
	if (shiftFrom == 0 && size ==0)
	{
            if (m_blocks.empty ())
                return T(m_extensionBit ? -1 : 0);
            else
                return T(m_blocks.at (0));
	}
	BitField cp (*this);
	cp >>= shiftFrom;
	if (size > 0)
	{
            BitField mask (true, size);
            cp &= mask;
	}
	if (cp.m_blocks.empty())
            return T(cp.m_extensionBit ? -1 : 0);
	return T(cp.m_blocks.at(0));
    }


    inline BitField BitField::operator~ () const
    {
	BitField bf (*this);
	for (std::vector<u_int64>::iterator it = bf.m_blocks.begin(), itEnd = bf.m_blocks.end(); it != itEnd; ++it)
	{
            (*it) = ~(*it);
	}
	bf.m_extensionBit = !m_extensionBit;
	return bf;
    }



    inline BitField BitField::operator>> (size_t shift) const
    {
	BitField bf (*this);
	return (bf>>=shift);
    }

    inline BitField BitField::operator<< (size_t shift) const
    {
	BitField bf (*this);
	return (bf<<=shift);
    }

    inline BitField& BitField::operator<<= (size_t shift)
    {
	if (m_blocks.empty ())
            return *this;

	size_t insertedBlocks = 0;
	while (shift >= 64)
	{
            m_blocks.insert (m_blocks.begin(), 0);
            shift -= 64;
            ++insertedBlocks;
	}

	size_t complementaryShiftLeft = 64 - shift;

	std::vector<u_int64>::iterator it      = m_blocks.begin()+insertedBlocks;
	std::vector<u_int64>::iterator itEnd   = m_blocks.end();
	u_int64 tmp = 0x0;
	u_int64 nextTmp = 0x0;
	while (it != itEnd)
	{
            u_int64& current = *it;
            if (current == 0)
            {
                ++it;
                continue;
            }
            nextTmp = current;
            // C++ standard: 
            // shifts larger than the number of bits in the variable to be shifted cause undefined behaivour
            // 
            if (complementaryShiftLeft >= sizeof (nextTmp)*8)
                nextTmp = 0;
            else
                nextTmp >>= complementaryShiftLeft;
            if (shift >= sizeof (current)*8)
                current = 0;
            else
                current <<= shift;
            current |= tmp;
            tmp = nextTmp;
            ++it;
	}
	if (tmp != 0)
	{
            if (m_extensionBit)
            {
                u_int64 filler = u_int64(ONES64) << shift;
                tmp |= filler;
            }
            m_blocks.push_back (tmp);
	}
	return *this;
    }

    inline BitField& BitField::operator>>= (size_t shift)
    {
	if (m_blocks.empty ())
            return *this;

	while (shift > 64)
	{
            m_blocks.erase (m_blocks.begin());
            if (m_blocks.empty())
                return *this;
            shift -= 64;
	}
	
	std::vector<u_int64>::iterator itBegin = m_blocks.begin();
	std::vector<u_int64>::iterator it      = m_blocks.end();
	u_int64 tmp = 0x0;
	u_int64 nextTmp = 0x0;
	u_int64 complementaryShiftRight = 64 - shift;
	while (itBegin != it)
	{
            --it;
            u_int64& current = *it;
            nextTmp = current;
            nextTmp <<= complementaryShiftRight;
            current >>= shift;
            current |= tmp;
            tmp = nextTmp;
	}
	if (m_blocks.back() == 0)
            m_blocks.erase (m_blocks.end() -1);

	if (m_extensionBit)
	{
            u_int64 filler = ONES64 << complementaryShiftRight;
            m_blocks.back () |= filler;
	}

	return *this;
    }

    inline BitField BitField::operator|  (const BitField& other) const
    {
	BitField bf (*this);
	return (bf|=other);
    }

    inline BitField& BitField::operator|= (const BitField& other)
    {
	if (other.m_extensionBit)
	{
            if (m_extensionBit)
            {
                if (other.blocks () < blocks())
                {
                    m_blocks.erase (m_blocks.begin() + other.blocks(), m_blocks.end());
                }
            }
            else // (!m_extensionBit)
            {
                if (other.blocks () > blocks())
                {
                    m_blocks.insert (m_blocks.end(), other.blocks () - blocks(), 0);
                }
                else if (other.blocks () < blocks())
                {
                    m_blocks.erase (m_blocks.begin() + other.blocks(), m_blocks.end());
                }
            }
	}
	else
	{
            if (!m_extensionBit && other.blocks () > blocks())
            {
                m_blocks.insert (m_blocks.end(), other.blocks() - blocks(), 0);
            }
	}
	std::vector<u_int64>::const_iterator itOther = other.m_blocks.begin(), itOtherEnd = other.m_blocks.end();
	std::vector<u_int64>::iterator it = m_blocks.begin(), itEnd = m_blocks.end();
	for ( ; itOther != itOtherEnd && it != itEnd; ++itOther, ++it)
	{
            (*it) |= (*itOther);
	}
	
	m_extensionBit |= other.m_extensionBit;

	if (m_blocks.empty())
            return *this;

	int index = m_blocks.size () - 1;
	while (index>=0)
	{
            if (m_blocks.at (index) != (m_extensionBit ? ONES64 : 0))
                break;

            m_blocks.erase (m_blocks.end()-1);
            if (index == 0)
                break;

            --index;
	}
	return *this;
    }


    inline BitField BitField::operator&  (const BitField& other) const
    {
	BitField bf (*this);
	return (bf&=other);
    }

    inline BitField& BitField::operator&= (const BitField& other)
    {
	if (other.m_extensionBit)
	{
            if (m_extensionBit)
            {
                if (other.blocks () > blocks())
                {
                    m_blocks.insert (m_blocks.end(), other.blocks() - blocks(), ONES64);
                }
            }
	}
	else
	{
            if (blocks () > other.blocks())
            {
                m_blocks.erase (m_blocks.begin() + other.blocks(), m_blocks.end());
            }
            else if (other.blocks () > blocks())
            {
                m_blocks.insert (m_blocks.end(), other.blocks() - blocks(), 0x0);
            }
	}


	if (m_blocks.empty ())
            return *this;

	std::vector<u_int64>::const_iterator itOther = other.m_blocks.begin(), itOtherEnd = other.m_blocks.end();
	std::vector<u_int64>::iterator it = m_blocks.begin(), itEnd = m_blocks.end();
	for ( ; itOther != itOtherEnd && it != itEnd; ++itOther, ++it)
	{
            (*it) &= (*itOther);
	}

	m_extensionBit &= other.m_extensionBit;

	int index = (int)m_blocks.size () - 1;
	while (index>=0)
	{
            if (m_blocks.at (index) != (m_extensionBit ? ONES64 : 0))
                break;

            m_blocks.erase (m_blocks.end()-1);
            if (index == 0)
                break;

            --index;
	}
	return *this;
    }




//inline bool BitField::checkSingleFlag(int index) const
//{
//    if (index < 64)
//    {
//        return (m_baseBitField & (M_one64 << index)) != 0;
//    }
//
//   //int i = (index / 64) - 1;
//    //int j = index % 64;
//    //faster version below
//    int i = (index >> 6) - 1;
//    int j = index & 0x0000003f;
//
//    if (i < (int) m_additionalBitFields.size())
//        return (m_additionalBitFields[i] & (M_one64 << j)) != 0;
//
//    return false;
//}
//
//inline bool BitField::empty() const
//{
//    if ( m_baseBitField != 0 )
//        return false;
//    
//	for (std::vector<u_int64>::const_iterator it = m_additionalBitFields.begin(), itEnd = m_additionalBitFields.end(); it != itEnd; ++it)
//    {
//        if ((*it) != 0)
//            return false;
//    }
//
//    return true;
//}
//
//
//inline bool BitField::full() const
//{
//    if ( m_baseBitField != CU64(0xffffffffffffffff) )
//        return false;
//    
//	for (std::vector<u_int64>::const_iterator it = m_additionalBitFields.begin(), itEnd = m_additionalBitFields.end(); it != itEnd; ++it)
//    {
//        if ((*it) != CU64(0xffffffffffffffff))
//            return false;
//    }
//
//    return true;
//}
//
//
//inline bool BitField::checkMatch(const BitField other) const
//{
//    if ((m_baseBitField & other.m_baseBitField) != 0)
//        return true;
//
//    for( size_t i = 0; i < m_additionalBitFields.size(); i++)
//    {
//        if (i >= other.m_additionalBitFields.size())
//            return false;                   //_assume 0 on all missing vector elements
//        if ((m_additionalBitFields[i] & other.m_additionalBitFields[i]) != 0)
//            return true;
//    }
//
//    return false;
//}
//
//inline void BitField::mergeAnd(const BitField other)
//{
//    m_baseBitField &= other.m_baseBitField;
//
//    for( size_t i = 0; i < m_additionalBitFields.size(); i++)
//    {
//        if (i < other.m_additionalBitFields.size() )
//            m_additionalBitFields[i] &= other.m_additionalBitFields[i];
//        else
//            m_additionalBitFields[i] = 0;              //if the other is smaller, zeros are assumed at the end
//    }
//}
//
//
//inline void BitField::mergeOr(const BitField other)
//{
//    m_baseBitField |= other.m_baseBitField;
//
//    for( size_t i = 0; i < m_additionalBitFields.size(); i++)
//    {
//        if (i < other.m_additionalBitFields.size() )
//            m_additionalBitFields[i] |= other.m_additionalBitFields[i];
//        else
//            m_additionalBitFields[i] = 0;              //if the other is smaller, zeros are assumed at the end
//    }
//}
//
////keep only flag index standing
//inline void BitField::setSingleFlag(int index)
//{
//    if (index < 64)
//        m_baseBitField = M_one64 << index;
//    else
//        m_baseBitField = 0;
//
//    int i = (index >> 6) - 1;
//    int j = index & 0x0000003f;
//    
//    if (i < 0) return;
//
//    //clear all indices
//    for( size_t k = 0; k < m_additionalBitFields.size(); k++)
//        m_additionalBitFields[k] = 0;
//
//    //add missing indices
//    while(i >= (int) m_additionalBitFields.size()) 
//        m_additionalBitFields.push_back(0);
//    
//    m_additionalBitFields[i] |= (M_one64 << j); 
//}
//
////add a flag
//inline void BitField::addSingleFlag(int index)
//{
//    if (index < 64)
//    {
//        m_baseBitField |= M_one64 << index;
//        return;
//    }
//   
//    //int i = (index / 64) - 1;
//    //int j = index % 64;
//    //faster version below
//    int i = (index >> 6) - 1;
//    int j = index & 0x0000003f;
//    
//    if (i < 0) return;
//    
//    while(i >= (int) m_additionalBitFields.size()) 
//        m_additionalBitFields.push_back(0);
//    
//    m_additionalBitFields[i] |= (M_one64 << j);    
//    
//}
//

} // namespace IKS
#endif

