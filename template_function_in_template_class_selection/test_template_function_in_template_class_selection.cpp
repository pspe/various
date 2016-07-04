
template<typename T> 
struct COMPILERPRINT
{ 
    operator char() { return sizeof(T) + 256; }  //always overflow
};
//if you doubt, you can use UCHAR_MAX +1 instead of 256, to ensure overflow.

template<typename F, typename S> 
struct COMPILERPRINT_COMPARE
{ 
    operator char() { return sizeof(F) + sizeof(S) + 256; }  //always overflow
};
//if you doubt, you can use UCHAR_MAX +1 instead of 256, to ensure overflow.

template <typename T> void FNC_COMPILERPRINT(T& t) { char(COMPILERPRINT<T>()); }
template <typename F, typename S> void FNC_COMPILERPRINT_COMPARE(F& f, S& s) { char(COMPILERPRINT_COMPARE<F,S>()); }


class NullType {};


template <class T, class U>
struct TypeList
{
    typedef T Head;
    typedef U Tail;
};



template <class T>
struct Holder
{
    typedef T types;
    typedef typename T::Head value_type;
    typedef Holder<typename T::Tail> next_type;

    value_type m_value;
    next_type  m_next;

    Holder(const value_type& initValue, const next_type& initNext)  : m_value (initValue), m_next (initNext) {}
    Holder(const value_type& initValue)  : m_value (initValue) {}
    Holder()  {}
};

template <>
struct Holder<NullType>
{
    typedef NullType value_type;
    typedef NullType next_type;

    value_type m_value;
    next_type  m_next;

    Holder(const value_type& initValue, const next_type& initNext)  : m_value (initValue), m_next (initNext) {}
    Holder(const value_type& initValue)  : m_value (initValue) {}
    Holder()  {}
};



#define TYPES_1(T1) TypeList<T1,NullType>
#define TYPES_2(T1, T2) TypeList<T1, TYPES_1(T2) >
#define TYPES_3(T1, T2, T3) TypeList<T1, TYPES_2(T2, T3) >
#define TYPES_4(T1, T2, T3, T4) TypeList<T1, TYPES_3(T2, T3, T4) >



template <typename TY>
struct X
{
    typedef X<typename TY::Tail> Next;

    template <typename H>
    void test (H& source)
    {
	Next next;
	next.test (source.m_next);
    }

};

template <>
struct X<NullType>
{
    void test (Holder<NullType>& source)
    {
	return;
    }
};




    int main()
    {
	typedef TYPES_3(char,int,double) TY;
	Holder<TY> holder;

	X<TY> transform;
	transform.test(holder);
    }
