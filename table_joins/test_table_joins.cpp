#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

#undef NDEBUG 
//#define DEBUG 1
#include <cassert>

#ifdef DEBUG
#define DBGMSG(s) {s}
#else
#define DBGMSG(s) {}
#endif

// template<size_t size> 
// struct overflow{ operator char() { return size + 256; } }; //always overflow
// //if you doubt, you can use UCHAR_MAX +1 instead of 256, to ensure overflow.

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

// =============== DEBUGGING HELPERS
//template <typename T> void FNC_COMPILERPRINT(T& t) { char(COMPILERPRINT<T>()); }
//template <typename F, typename S> void FNC_COMPILERPRINT_COMPARE(F& f, S& s) { char(COMPILERPRINT_COMPARE<F,S>()); }
// =============== DEBUGGING HELPERS



template <bool Condition, typename Then, typename Else>
struct IfThenElse;

template <typename Then, typename Else>
struct IfThenElse<true, Then, Else>
{
    typedef Then return_type;
};

template <typename Then, typename Else>
struct IfThenElse<false, Then, Else>
{
    typedef Else return_type;
};

template <int v>
struct SpecifyInt  { enum { value = v }; };

template <bool v>
struct SpecifyBool  { enum { value = v }; };


template <typename T>
struct SpecifyType { typedef T type; };


// convert an expression of type T to an
// expression of type type2type<T>
template< class T >
SpecifyType< T > encode_type ( T const & t) 
{
    return SpecifyType< T >();
}

// convertible to SpecifyType<T> for any T
struct any_type
{
    template< class T >
    operator SpecifyType< T > () const
    {
	return SpecifyType< T >();
    }
};

// convert an expression of type T to an
// expression of type SpecifyType<T> without
// evaluating the expression
#define ENCODED_TYPEOF( container ) ( true ? any_type() : encode_type( container ) )






class NullType 
{ 
public: 
    typedef NullType value_type;
    typedef NullType next_type;

    static NullType m_next;
    static NullType m_value;
};




template <class T, class U>
struct TypeList
{
    typedef T Head;
    typedef U Tail;
};


namespace TT
{
// =========== length
    template <class TList> struct Length;
    template <> struct Length<NullType>
    {
	enum { value = 0 };
    };
    template <class T, class U>
    struct Length< TypeList<T, U> >
    {
	enum { value = 1 + Length<U>::value };
    };
}


template <typename S, typename T>
S& operator<<(S& s, const T* val)
{
    s << "ptr" << val;
    return s;
}

template <typename S, typename T>
S& operator<<(S& s, const T& val)
{
    s << '_';
    return s;
}



template <class T>
struct Holder
{
    typedef T types;
    typedef typename T::Head value_type;
    typedef Holder<typename T::Tail> next_type;

    value_type m_value;
    next_type  m_next;

    Holder(const Holder<types>& init) : m_value (init.m_value), m_next(init.m_next) { DBGMSG(std::cout << "(H " << m_value << "|" << m_next.m_value << ")" << std::flush;) }
    Holder(const value_type& initValue, const next_type& initNext)  : m_value (initValue), m_next (initNext)  {	DBGMSG(std::cout << "(VN " << m_value << "|" << m_next.m_value << " )" << std::flush;) }
    Holder(const value_type& initValue)  : m_value (initValue) { DBGMSG(std::cout << "(V " << m_value << " )" << std::flush;) }
    Holder()  { DBGMSG(std::cout << "(EMPTY)" << std::endl;) }
};

template <>
struct Holder<NullType>
{
    typedef NullType value_type;
    typedef NullType next_type;

    value_type m_value;
    next_type  m_next;

    Holder(const Holder<NullType>& init) {DBGMSG(std::cout << "(NEMPTY)" << std::endl;)}
    Holder(const value_type& initValue, const next_type& initNext)  {DBGMSG(std::cout << "(NEMPTY)" << std::endl;)}
    Holder(const value_type& initValue)   {DBGMSG(std::cout << "(NEMPTY)" << std::endl;)}
    Holder()  {DBGMSG(std::cout << "(NEMPTY)" << std::endl;)}
};





#define TYPES_1(T1) TypeList<T1,NullType>
#define TYPES_2(T1, T2) TypeList<T1, TYPES_1(T2) >
#define TYPES_3(T1, T2, T3) TypeList<T1, TYPES_2(T2, T3) >
#define TYPES_4(T1, T2, T3, T4) TypeList<T1, TYPES_3(T2, T3, T4) >



#define VALTN_1(Ta, V0) Holder< Ta>(V0)
#define VALTN_2(Tb, V0, V1) Holder< Tb>(V0, VALTN_1(Tb::Tail, V1))
#define VALTN_3(Tc, V0, V1, V2) Holder< Tc>(V0, VALTN_2(Tc::Tail, V1, V2))

#define VAL_1(Ta, V0) Holder<Ta>(V0)
#define VAL_2(Tb, V0, V1) Holder<Tb>(V0, VALTN_1(Tb::Tail, V1))
#define VAL_3(Tc, V0, V1, V2) Holder<Tc>(V0, VALTN_2(Tc::Tail, V1, V2))


namespace TT
{
// =========== type at
    template <class TList, unsigned int index> struct TypeAt;
         
    template <class Head, class Tail>
    struct TypeAt<TypeList<Head, Tail>, 0>
    {
	typedef TypeList<Head, Tail> input_type;
	typedef Head value_type;
	typedef Tail next_type;
    };
 
    template <class T, unsigned int i>
    struct TypeAt
    {
	typedef T input_type;
	typedef T value_type;
	typedef NullType next_type;
    };
 
    template <class Head, class Tail, unsigned int i>
    struct TypeAt<TypeList<Head, Tail>, i>
    {
	typedef TypeList<Head, Tail> input_type;
	typedef typename TypeAt<Tail,i-1>::value_type value_type;
	typedef typename TypeAt<Tail,i-1>::next_type  next_type;
    };

    template <class Head, class Tail, unsigned int i>
    struct TypeAt<Holder<TypeList<Head, Tail> >, i>
    {
	typedef TypeList<Head, Tail> input_type;
	typedef typename TypeAt<input_type,i>::value_type value_type;
	typedef typename TypeAt<input_type,i>::next_type  next_type;
    };


// ============== append types
    template <class TList, class T> struct Append;

    template <> struct Append<NullType, NullType>
    {
	typedef NullType types;
    };

    template <class T> struct Append<NullType, T>
    {
	typedef TYPES_1(T) types;
    };

    template <class Head, class Tail>
    struct Append<NullType, TypeList<Head, Tail> > 
    {
	typedef TypeList<Head, Tail> types;
    };

    template <class Head, class Tail, class T>
    struct Append<TypeList<Head, Tail>, T>
    {
	typedef TypeList<Head, typename Append<Tail, T>::types> types;
    };

// =============== field

    template <class H, typename R>
    inline R& FieldHelper(H& obj, SpecifyType<R>, SpecifyInt<0>)
    {
    	return obj.m_value;
    }

    template <class H, typename R, int i>
    inline R& FieldHelper(H& obj, SpecifyType<R> tt, SpecifyInt<i>)
    {
    	typename H::next_type& subobj = obj.m_next;
    	return FieldHelper(subobj, tt, SpecifyInt<i - 1>());
    }



    template <int i, class H>
    typename TypeAt<H,i>::value_type& Field (H& obj)
    {
    	typedef typename TypeAt<H,i>::value_type value_type;
    	return FieldHelper(obj, SpecifyType<value_type>(), SpecifyInt<i>());
    }



// =============== connect



    template <typename T0, typename T1>
    struct Connect_t
    {
	typedef typename Append<typename T0::types,typename T1::types>::types types;
	typedef Holder<types> return_type;

	template <typename H0, typename H1>
	Holder<types> operator() (H0& holder0, H1& holder1)
	{
	    DBGMSG(std::cout << "[H0,H1]";)
		return Initialize (holder0, holder1);
	}

    private:

	template <typename H0> 
	Holder<typename TT::Append<typename H0::types,typename T1::types>::types> Initialize (H0& holder0,  Holder<typename T1::types>& holder1) 
	{
	    typedef typename TT::Append<typename H0::types,typename T1::types>::types return_type_types;
	    typedef Holder<return_type_types> return_type;

	    const typename return_type::value_type& value = Field<0>(holder0);
	    DBGMSG(std::cout << "[H0,T1 " << value << "]";)
		return return_type (value, Initialize (holder0.m_next,holder1));
	}


	Holder<typename T1::types> Initialize (Holder<NullType>& holder0, Holder<typename T1::types>& holder1) 
	{
	    typedef Holder<typename T1::types> return_type;
	    const typename return_type::value_type& value = Field<0>(holder1);
	    DBGMSG(std::cout << "[NULL,H1 " << value << "|" << holder1.m_next.m_value << "]";)
		return return_type (value, Initialize (holder1.m_next));
	}

	
	template <typename H1> 
	H1 Initialize (H1& holder1) 
	{
	    typedef Holder<typename H1::types> return_type;
	    const typename return_type::value_type& value = Field<0>(holder1);
	    DBGMSG(std::cout << "[H1 " << value << "]";)
		return return_type (value, Initialize (holder1.m_next));
	}

        Holder<NullType> Initialize (Holder<NullType>& holder1) 
	{
	    DBGMSG(std::cout << "[NULL]";)
		return Holder<NullType>();
	}

	
    };



// =============== transform


    

    template <typename T, template <typename> class TransPol>
    struct Transform_t
    {
    	typedef TypeList<typename TransPol<typename T::Head>::iterator_type, typename Transform_t<typename T::Tail,TransPol>::types > types;

	typedef typename T::Tail NextTypes;
	typedef Transform_t<NextTypes, TransPol> Next;


	template <typename H>
	Holder<types> operator() (H& source)
	{
	    Next transformNext;
	    TransPol<typename H::types::Head> policy;

	    return Holder<types>(policy(source.m_value), transformNext (source.m_next));
	}
    };

    template <template <typename> class TransPol>
    struct Transform_t<NullType, TransPol>
    {
    	typedef NullType types;

	Holder<NullType> operator() (Holder<NullType>& source)
	{
	    return Holder<NullType>();
	}
    };


    // 	template <typename H0, typename H1>
    // 	Holder<types> operator() (H0& holder0, H1& holder1)
    // 	{
    // 	    return Initialize (holder0, holder1);
    // 	}

    // private:

    // 	template <typename H0> 
    // 	Holder<typename TT::Append<typename H0::types,typename T1::types>::types> Initialize (H0& holder0,  Holder<typename T1::types>& holder1) 
    // 	{
    // 	    typedef typename TT::Append<typename H0::types,typename T1::types>::types return_type_types;
    // 	    typedef Holder<return_type_types> return_type;

    // 	    const typename return_type::value_type& value = Field<0>(holder0);
    // 	    return return_type (value, Initialize (holder0.m_next,holder1));
    // 	}


    // 	Holder<typename T1::types> Initialize (Holder<NullType>& holder0, Holder<typename T1::types>& holder1) 
    // 	{
    // 	    typedef Holder<typename T1::types> return_type;
    // 	    const typename return_type::value_type& value = Field<0>(holder1);
    // 	    return return_type (value, Initialize (holder1.m_next));
    // 	}

	
    // 	template <typename H1> 
    // 	H1 Initialize (H1& holder1) 
    // 	{
    // 	    typedef Holder<typename H1::types> return_type;
    // 	    const typename return_type::value_type& value = Field<0>(holder1);
    // 	    return return_type (value, Initialize (holder1.m_next));
    // 	}

    //     Holder<NullType> Initialize (Holder<NullType>& holder1) 
    // 	{
    // 	    return Holder<NullType>();
    // 	}

	
    // };





// =============== table

    template <typename T>
    struct Identity
    {
	typedef T table_type;

	template <typename U>
	U operator() (U& input) const
	{
	    return input;
	}
    };




    template <typename TSource, typename TDest>
    void CopyValues (const TSource& source, TDest& dest)
    {
	typedef TT::Transform_t<typename TDest::types, TT::Identity> Transform;
	Transform transform;
	dest = transform (source);
    }



    template <typename Ty>
    class Table_t
    {
    public:
	typedef Ty               types;
	typedef Table_t<Ty> table_type;

	Holder<types> m_fields;
	unsigned int  m_size;

	Table_t () : m_size(0) {}
	Table_t (const table_type&    other ) : m_fields (other.m_fields), m_size(other.m_size) {}
	Table_t (const Holder<types>& fields) : m_fields(fields), m_size(0) {}

	table_type clone() const
	{
	    return table_type (*this);
	}

	size_t size() const { return m_size; }
        void nextRow() { ++m_size; }
    };



// ================= field (on table)
    template <int i, class H>
    typename TypeAt<H,i>::value_type& Field (Table_t<H>& obj)
    {
    	return Field<i> (obj.m_fields);
    }
// ==================

#define TCNT(T) std::vector<T>







#define TABLE_COMMON(NAME,TL,N0,N1,N2,N3)	\
    class NAME : public TT::Table_t<TL> \
    { \
    public: \
	\
	NAME() : TT::Table_t<TL>() {}		\
\
	TT::TypeAt<types,0>::value_type& N0() { return TT::Field<0>(this->m_fields); } \
	TT::TypeAt<types,1>::value_type& N1() { return TT::Field<1>(this->m_fields); } \
	TT::TypeAt<types,2>::value_type& N2() { return TT::Field<2>(this->m_fields); } \
	TT::TypeAt<types,3>::value_type& N3() { return TT::Field<3>(this->m_fields); } \
\
        typedef TT::Transform_t<types, TT::GetIterator> IteratorHolder; \
        typedef TT::Transform_t<types, TT::GetInsertIterator> InsertIteratorHolder; \
	\
        template <typename HolderType> \
        class HolderFieldAccessor \
        { \
        public: \
	    HolderType m_holder; \
	    HolderFieldAccessor (HolderType holder) : m_holder(holder) \
	    { }			\
	    typename TT::TypeAt<TL,0>::value_type::value_type& N0() { return *TT::Field<0>(m_holder); } \
	    typename TT::TypeAt<TL,1>::value_type::value_type& N1() { return *TT::Field<1>(m_holder); } \
	    typename TT::TypeAt<TL,2>::value_type::value_type& N2() { return *TT::Field<2>(m_holder); } \
	    typename TT::TypeAt<TL,3>::value_type::value_type& N3() { return *TT::Field<3>(m_holder); } \
\
	    template <typename T> \
	    T N0(T value) { *TT::Field<0>(m_holder) = value; return value; } \
	    template <typename T> \
	    T N1(T value) { *TT::Field<1>(m_holder) = value; return value; } \
	    template <typename T> \
	    T N2(T value) { *TT::Field<2>(m_holder) = value; return value; } \
	    template <typename T> \
	    T N3(T value) { *TT::Field<3>(m_holder) = value; return value; } \
\
	    HolderFieldAccessor& operator++() { Increment(m_holder); return *this; } \
	    HolderFieldAccessor& operator++(int) \
	    { \
 	        HolderFieldAccessor current(*this);	\
		++(*this);				\
		return current; \
	    }			\
	    \
        private:			\
	    \
	    template <typename T> void Increment (T& holder, SpecifyInt<0>) { ++(TT::Field<0>(holder)); }\
	    template <typename T, int i> void Increment (T& holder, SpecifyInt<i>) { ++(TT::Field<i>(holder)); Increment (holder, SpecifyInt<i-1>()); } \
	    template <typename T> void Increment (Holder<T>& holder) { const int length = TT::Length<T>::value; Increment (holder, SpecifyInt<length-1>());}\
        }; \
\
\
    }



	// NAME() : TT::Table_t<types>() {}	
	// NAME(const table_type& other) : TT::Table_t<types>(other) {} 
        // NAME(const Holder<types>& fields) : TT::Table_t<types>(fields) {} 


//	    Table_t<typename TransformFirst::types> firstRunning (transformFirst (first.m_fields)); // ? holder or table_t ?




#define TABLE_1(NAME,T0,N0) TABLE_COMMON(NAME,TYPES_1(TCNT(T0)),N0,_f1_,_f2_,_f3_)
#define TABLE_2(NAME,T0,N0,T1,N1) TABLE_COMMON(NAME,TYPES_2(TCNT(T0),TCNT(T1)),N0,N1,_f2_,_f3_)
#define TABLE_3(NAME,T0,N0,T1,N1,T2,N2) TABLE_COMMON(NAME,TYPES_3(TCNT(T0),TCNT(T1),TCNT(T2)),N0,N1,N2,_f3_)
#define TABLE_4(NAME,T0,N0,T1,N1,T2,N2,T3,N3) TABLE_COMMON(NAME,TYPES_4(TCNT(T0),TCNT(T1),TCNT(T2),TCNT(T3)),N0,N1,N2,N3)


// =============== get table of iterators for containers in the table


    template <typename T>
    struct GetIterator
    {
	typedef typename T::iterator iterator_type;

	template <typename U>
	typename U::iterator operator() (U& input)
	{
	    return input.begin();
	}

	template <typename U>
	typename U::iterator end (U& input)
	{
	    return input.end();
	}
    };


    template <typename T>
    struct GetInsertIterator
    {
	typedef std::insert_iterator<T> iterator_type;

	template <typename U>
	typename std::insert_iterator<U> operator() (U& input) const 
	{
	    return std::insert_iterator<U>(input,input.end());
	}
    };



    

// =============== joins






    template <typename T0, typename T1, bool doInnerJoin>
    class Join_t
    {
    public:

	typedef T0 first_table;
	typedef T1 second_table;

	typedef typename T0::IteratorHolder TransformT0;
	typedef typename T1::IteratorHolder TransformT1;


	first_table&  m_leftTable;
	second_table& m_rightTable;


	Join_t(first_table& left, second_table& right)
	    : m_leftTable(left),
	      m_rightTable(right)
	{
	}

	template <class SelectType, class WhereType>
	SelectType& operator() (SelectType& select, WhereType& where)
	{

	    TransformT0 transformT0;
	    TransformT1 transformT1;

	    typedef typename T0::template HolderFieldAccessor<Holder<typename TransformT0::types> > AccessorT0Entry;
	    typedef typename T1::template HolderFieldAccessor<Holder<typename TransformT1::types> > AccessorT1Entry;

	    int pos0 = 0;
	    int pos0End = m_leftTable.m_size;

//	    std::cout << "select --- size table 0 = " << pos0End << std::endl;
	    AccessorT0Entry accT0(transformT0 (m_leftTable.m_fields));
	    while (pos0<pos0End)
	    {
		int pos1 = 0;
		int pos1End = m_rightTable.m_size;
//		std::cout << "select --- table0 pos = " << pos0 << " , size table 1 = " << pos1End << std::endl;
		AccessorT1Entry accT1(transformT1(m_rightTable.m_fields));
		bool hasFoundMatch = false;
		while (pos1<pos1End)
		{
		    if (where.operator() (accT0,accT1))
		    {
			hasFoundMatch = true;
			select.operator() (accT0,accT1);
			++select.m_size;
		    }

		    ++accT1;
		    ++pos1;
		}
		if (!hasFoundMatch)
		{
		    SelectNonMatched (select, accT0, SpecifyBool<doInnerJoin>());
		}
		++accT0;
		++pos0;
	    }
	    return select;
	}
    private:
	template <typename Select, typename Accessor>
	inline void SelectNonMatched (Select& /*select*/, Accessor& /*accessor*/, SpecifyBool<true>)
	{
	    // do nothing
	}
	template <typename Select, typename Accessor>
	inline void SelectNonMatched (Select& select, Accessor& accessor, SpecifyBool<false>)
	{
	    select.operator() (accessor);
	    ++select.m_size;
	}
    }; 


    template <typename T0,typename T1>
    Join_t<T0,T1,true> InnerJoin (T0& tab0, T1& tab1)
    {
	return Join_t<T0,T1,true>(tab0,tab1);
    }

    template <typename T0,typename T1>
    Join_t<T0,T1,false> LeftJoin (T0& tab0, T1& tab1)
    {
	return Join_t<T0,T1,false>(tab0,tab1);
    }


// =============== select


// =============== group by

//     template <typename T0>
//     class GroupBy_t
//     {
//     public:

// 	typedef T0 first_table;

// 	typedef typename T0::IteratorHolder TransformT0;

// 	first_table& m_table;


// 	Group_t(first_table& table)
// 	    : m_table(table)
// 	{
// 	}

// 	template <class SelectType, class GroupType, class HavingType>
// 	SelectType& operator() (SelectType& select, GroupType& group, HavingType& having)
// 	{
// 	    TransformT0 transformT0;

// 	    typedef typename T0::template HolderFieldAccessor<Holder<typename TransformT0::types> > AccessorT0Entry;

// 	    int pos0 = 0;
// 	    int pos0End = m_table.m_size;

// //	    std::cout << "select --- size table 0 = " << pos0End << std::endl;
// 	    AccessorT0Entry accT0(transformT0 (m_table.m_fields));
// 	    while (pos0<pos0End)
// 	    {
// 		int pos1 = 0;
// 		int pos1End = m_table.m_size;
// //		std::cout << "select --- table0 pos = " << pos0 << " , size table 1 = " << pos1End << std::endl;
// 		AccessorT1Entry accT1(transformT1(m_rightTable.m_fields));
// 		bool hasFoundMatch = false;
// 		while (pos1<pos1End)
// 		{
// 		    if (where.operator() (accT0,accT1))
// 		    {
// 			hasFoundMatch = true;
// 			select.operator() (accT0,accT1);
// 			++select.m_size;
// 		    }

// 		    ++accT1;
// 		    ++pos1;
// 		}
// 		if (!hasFoundMatch)
// 		{
// 		    SelectNonMatched (select, accT0, SpecifyBool<doInnerJoin>());
// 		}
// 		++accT0;
// 		++pos0;
// 	    }
// 	    return select;
// 	}
//     private:
// 	template <typename Select, typename Accessor>
// 	inline void SelectNonMatched (Select& /*select*/, Accessor& /*accessor*/, SpecifyBool<true>)
// 	{
// 	    // do nothing
// 	}
// 	template <typename Select, typename Accessor>
// 	inline void SelectNonMatched (Select& select, Accessor& accessor, SpecifyBool<false>)
// 	{
// 	    select.operator() (accessor);
// 	    ++select.m_size;
// 	}
//     }; 

// =============== group by

    // struct MySelect 
    // {
    // 	template <typename T0, typename T1>
    // 	void operator() (T0& tab0, T1& tab1)
    // 	{
    // 	    std::cout << "tab0<0> = " << (*TT::Field<0>(tab0)) << " tab0<1> = " << (*TT::Field<1>(tab0)) << "  ";
    // 	    std::cout << "tab1<0> = " << (*TT::Field<0>(tab1)) << " tab1<1> = " << (*TT::Field<1>(tab1)) << std::endl;
    // 	}
    // };




    TABLE_2(MySelectToTable_table,double,age,double,agePerson);
    class MySelectToTable : public MySelectToTable_table
    {
	InsertIteratorHolder  m_transformer;
	typedef HolderFieldAccessor<Holder<InsertIteratorHolder::types> > AccessorEntry;
	AccessorEntry select;

    public:
	MySelectToTable() : MySelectToTable_table() , m_transformer() , select(m_transformer (m_fields)) {}

	template <typename T0, typename T1>
	void operator() (T0& left, T1& right)
	{
	    std::cout << "S: left.age() = " << (left.age()) << " left.value() = " << (left.value()) << "  ";
	    std::cout << "S: right.person() = " << (right.person()) << " right.value() = " << (right.value()) << std::endl;

//	    left.age(44);
	    select.age(left.age());
	    select.agePerson(left.age() * right.person());
//	    ++m_size;
	}
    };



//#define TESTTEST(TYL,N0,N1,N2,N3) TABLE_COMMON("myname",TYL,N0,N1,N2,N3)
//#define TESTTEST(NAME,T0,T1,N0,N1) TABLE_COMMON(NAME,TYPES_2(TCNT(T0),TCNT(T1)),N0,N1,_f2_,_f3_)
#define TESTTEST(NAME,TYPELIST,N0,N1) TABLE_COMMON(NAME,TYPELIST,N0,N1,_f2_,_f3_)

    TABLE_4(mytable,double,xxx,int,yyy,double,zzz,char,vvv);    
    typedef TYPES_2(TCNT(double),TCNT(int)) mytype;
    TESTTEST(myname,mytype,aaa,bbb);
//    TESTTEST(myname,double,int,age,yourage);


#define SELECT_COMMON(NAME,TL,N0,N1,N2,N3)  \
    typedef TL _typelist_##NAME; \
    TABLE_COMMON(_table_##NAME,_typelist_##NAME,N0,N1,N2,N3);	\
\
    class NAME : public _table_##NAME \
    { \
    InsertIteratorHolder  m_transformer;				\
	typedef HolderFieldAccessor<Holder< InsertIteratorHolder::types> > AccessorEntry; \
	AccessorEntry select; \
 \
    public: \
	NAME() : _table_##NAME() , m_transformer() , select(m_transformer (m_fields)) {} \
\




#define SELECT_1(NAME,T0,N0) SELECT_COMMON(NAME,TYPES_1(TCNT(T0)),N0,_f1_,_f2_,_f3_)
#define SELECT_2(NAME,T0,N0,T1,N1) SELECT_COMMON(NAME,TYPES_2(TCNT(T0),TCNT(T1)),N0,N1,_f2_,_f3_)
#define SELECT_3(NAME,T0,N0,T1,N1,T2,N2) SELECT_COMMON(NAME,TYPES_3(TCNT(T0),TCNT(T1),TCNT(T2)),N0,N1,N2,_f3_)
#define SELECT_4(NAME,T0,N0,T1,N1,T2,N2,T3,N3) SELECT_COMMON(NAME,TYPES_4(TCNT(T0),TCNT(T1),TCNT(T2),TCNT(T3)),N0,N1,N2,N3)



// =============== where

    struct MyWhere
    {
	template <typename T0, typename T1>
	bool operator() (T0& left, T1& right)
	{
	    if ( left.value() == right.value() )
		return true;
	    return false;
	}
    };



#define SELECT(NAME) \
    struct NAME \
    { \

#define WHERE(NAME) \
    struct NAME \
    { \

#define LEFT_RIGHT \
        template <typename T0, typename T1>  \
	bool operator() (T0& left, T1& right)  \

#define LEFT \
        template <typename T0>  \
	bool operator() (T0& left)  \

#define END \
    }; \






}


WHERE(MyOtherWhere)
LEFT_RIGHT
{
    return left.value() == right.value();
}
END



template <typename T>
class Sum 
{
    T m_value;
public:
    Sum() : m_value(T(0)) {}

    operator T() { return m_value; }
    T operator()(T value) { m_value += value; return m_value; }
};


class Count 
{
    size_t m_value;
public:
    Count() : m_value(0) {}

    operator size_t() { return m_value; }
    size_t operator()() { ++m_value; return m_value; }
};


SELECT_3(TestSelect,double,age,double,agePerson,double*,agePtr)
Sum<double> SumAgePerson;
Count       CountAgePerson;
LEFT_RIGHT
{
    std::cout << "S: left.age() = " << (left.age()) << " left.value() = " << (left.value()) << "  ";
    std::cout << "S: right.person() = " << (right.person()) << " right.value() = " << (right.value()) << std::endl;

    select.age(left.age());
//    select.agePerson(right.value());
    select.agePtr(&(left.age()));

//    select.agePerson (SumAgePerson(right.value()));
    std::cout << "sum: " << SumAgePerson(right.value()) << "   count: " << CountAgePerson();
    std::cout << "/: " << ((double)SumAgePerson) << std::endl;
    select.agePerson (SumAgePerson/CountAgePerson);
//    select.agePerson (CountAgePerson());

//    left.age(44);

// //    FNC_COMPILERPRINT (TT::Field<2>(this->m_fields).back());
//     std::cout << "  s<2> = " << (*TT::Field<2>(this->m_fields).back()) << "  size: " << TT::Field<2>(this->m_fields).size() << std::endl;
//     std::cout << "  s<2>(0) = " << (*TT::Field<2>(this->m_fields).at(0)) << "  size: " << TT::Field<2>(this->m_fields).size() << std::endl;
//     std::cout << "  ptr = " << ((const void*)&left.age()) << std::endl;
     return true;
}

LEFT
{
    select.age(left.age());
    select.agePerson(0);
    select.agePtr(&left.age());
    return true;
}
END




TABLE_3(Mine,double,age,int,value,double,number);
TABLE_2(Other,double,person,int,value);





int main()
{

//    Join (Select<Tab0,Tab1,Tab2>("fieldA","fieldB","fieldC",Compute()), Where(
//    Join (Select<Tab0,Tab1,Tab2>("fieldA","fieldB","fieldC",Compute()), Where(



    typedef Holder<TYPES_3(char, int, double)> TB;
//    typedef Holder<TYPES_4(int, int, double, int)> TB;
//    typedef TYPES_3(char, int, double) TB;
    TB tb;
    TT::TypeAt<TB,1>::value_type val = 10;
    std::cout << val << std::endl;

    TT::Field<1>(tb) = 3;
    std::cout << " field 1 : " << TT::Field<1>(tb) << std::endl;
    TT::TypeAt<TB,1>::value_type f1 = TT::Field<1>(tb);

    std::cout << " field 1 (copied) : " << f1 << std::endl;
    TT::Field<2>(tb) = 8;
    std::cout << " field 2  : " << (TT::Field<2>(tb)) << std::endl;

//    std::cout << " field 3 : " << TT::Field<3>(tb) << std::endl;
 
    typedef Holder<TYPES_2(double, double)> TB2;

    TB2 tb2;

    // TBX tbx;
    // TT::Field<4>(tbx) = 3.8;
    // std::cout << "tbx field 4: " << TT::Field<4>(tbx) << std::endl;

    // char(COMPILERPRINT<typename TT::TypeAt<TB,0>::value_type>());
    // char(COMPILERPRINT<typename TT::TypeAt<TB,1>::value_type>());
    // char(COMPILERPRINT<typename TT::TypeAt<TB,2>::value_type>());


    TT::Field<0>(tb) = TT::TypeAt<TB,0>::value_type(1.1);
    TT::Field<1>(tb) = TT::TypeAt<TB,1>::value_type(2.2);
    TT::Field<2>(tb) = 3.3;

    TT::Field<0>(tb2) = 4.4;
    TT::Field<1>(tb2) = 5.5;

    typedef TT::Append<TB::types,TB2::types>::types TBX;
    typedef TT::Connect_t<TB,TB2> CType;
    CType connect;

    std::cout << "----- connect --- " << std::endl;
    CType::return_type tbx = connect (tb,tb2);

    std::cout << "----------------------- field test" << std::endl;
    std::cout << "F0 : " << TT::Field<0>(tbx) << std::endl;
    std::cout << "F1 : " << TT::Field<1>(tbx) << std::endl;
    std::cout << "F2 : " << TT::Field<2>(tbx) << std::endl;
    std::cout << "F3 : " << TT::Field<3>(tbx) << std::endl;
    std::cout << "F4 : " << TT::Field<4>(tbx) << std::endl;

    std::cout << "----------------------- refill field test" << std::endl;
    TT::Field<0>(tbx) = 3;
    TT::Field<1>(tbx) = 4;
    TT::Field<2>(tbx) = 5;
    TT::Field<3>(tbx) = 6;
    TT::Field<4>(tbx) = 7;
    std::cout << "F0 : " << TT::Field<0>(tbx) << std::endl;
    std::cout << "F1 : " << TT::Field<1>(tbx) << std::endl;
    std::cout << "F2 : " << TT::Field<2>(tbx) << std::endl;
    std::cout << "F3 : " << TT::Field<3>(tbx) << std::endl;
    std::cout << "F4 : " << TT::Field<4>(tbx) << std::endl;


    typedef TYPES_1(int) T1;
    VAL_1(T1,3);

    typedef TYPES_2(int, double) T2;
    VAL_2(T2, 3, 2.2);

    typedef TYPES_3(int, double,char) T3;
    Holder<T3>(3, Holder<T3::Tail>(2.2, Holder<T3::Tail::Tail>(3)));
    VAL_3(T3, 3, 2.2, 3);




    std::vector<Holder<T3> > vec;
    vec.push_back (VAL_3(T3, 1,2,3));
    vec.push_back (VAL_3(T3, 2,3,4));
    vec.push_back (VAL_3(T3, 8,7,6));
 
    for (std::vector<Holder<T3> >::iterator it = vec.begin(), itEnd = vec.end(); it != itEnd; ++it)
    {
	Holder<T3>& holder = (*it);
	std::cout << "f0= " << TT::Field<0>(holder) << " f1= " << TT::Field<1>(holder) << " f2= " << TT::Field<2>(holder) << std::endl;
    }



    typedef TYPES_2(int,double) TVALS;

    {
	std::cout << "using holder" << std::endl;
	Holder<TVALS> tvalsh;
	TT::Field<0>(tvalsh) = 1;
	TT::Field<1>(tvalsh) = 2;
	Holder<TVALS> tvalsCph(tvalsh);
	std::cout << "tvals,0: " << (TT::Field<0>(tvalsCph)) << std::endl;
	std::cout << "tvals,1: " << (TT::Field<1>(tvalsCph)) << std::endl;
	assert ((TT::Field<0>(tvalsh))==1);
	assert ((TT::Field<1>(tvalsh))==2);
    }

//    {
//	std::cout << "using table" << std::endl;
//	TT::Table_t<TVALS> tvals;
//	TT::Field<0>(tvals) = 3;
//	TT::Field<1>(tvals) = 4;
//	TT::Table_t<TVALS> tvalsCp(tvals);
//	std::cout << "tvals,0: " << TT::Field<0>(tvalsCp) << std::endl;
//	std::cout << "tvals,1: " << TT::Field<1>(tvalsCp) << std::endl;
//    }


    typedef TYPES_2(std::vector<int>, std::vector<double>) TV2;
    typedef Holder<TV2> HTV2;


    HTV2 htv2;
    typedef TT::Transform_t<TV2, TT::GetInsertIterator> HTV2TransformInsert;
    HTV2TransformInsert transformInsert;
    Holder< HTV2TransformInsert::types> iteratorsInsert = transformInsert (htv2);
//    FNC_COMPILERPRINT(iteratorsInsert);

    *TT::Field<0>(iteratorsInsert) = 3;
    *TT::Field<0>(iteratorsInsert) = 4;
    *TT::Field<1>(iteratorsInsert) = 8;

    TT::Table_t< HTV2TransformInsert::types> iteratorsInsertTbl = transformInsert (htv2);
    *TT::Field<1>(iteratorsInsertTbl.m_fields) = 9;




    typedef TT::Transform_t<TV2, TT::GetIterator> HTV2Transform;
    HTV2Transform transform;
    Holder< HTV2Transform::types> iterators = transform (htv2);
//    FNC_COMPILERPRINT(iterators);

    ENCODED_TYPEOF(TT::Field<0>(htv2).begin()); //::type myIt = TT::Field<0>(htv2).begin();


    std::cout << "field 0 size: " << TT::Field<0>(htv2).size() << std::endl;
    std::cout << "field 1 size: " << TT::Field<1>(htv2).size() << std::endl;

    for (TT::TypeAt<HTV2,0>::value_type::const_iterator it = TT::Field<0>(htv2).begin(), itEnd = TT::Field<0>(htv2).end(); it != itEnd; ++it)
    {
	std::cout << "inserted (0) " << (*it) << std::endl;
    }

    for (TT::TypeAt<HTV2,1>::value_type::const_iterator it = TT::Field<1>(htv2).begin(), itEnd = TT::Field<1>(htv2).end(); it != itEnd; ++it)
    {
	std::cout << "inserted (1) " << (*it) << std::endl;
    }
    
    std::cout << "---" << std::endl;



//     typedef TT::Table_t<TB>::table_type TABT;
//     TABT tabt;
//     TT::Field<0>(tabt.m_fields) = 0;
//     TT::Field<1>(tabt.m_fields) = 1;
//     TT::Field<2>(tabt.m_fields) = 2;

//     TABT tabtCopy(tabt);

//     std::cout << "TABTCOPY0 : " << TT::Field<0>(tabtCopy.m_fields) << std::endl;
//     std::cout << "TABTCOPY1 : " << TT::Field<1>(tabtCopy.m_fields) << std::endl;
//     std::cout << "TABTCOPY2 : " << TT::Field<2>(tabtCopy.m_fields) << std::endl;

//     typedef typename TT::TypeAt<TABT::types,2>::value_type inboundstype;
// //    inboundstype yyy = 7;

//     typedef typename TT::TypeAt<TABT::types,8>::value_type outofboundstype;
//     outofboundstype xxx = NullType();

    

    typedef TYPES_2(std::vector<int>, std::vector<double>) TCTypes;
    typedef TT::Table_t<TCTypes>::table_type TC;
    TC tc0;
    TC tc1;
    typedef TT::Transform_t<TCTypes, TT::GetInsertIterator> TCInsert0;
    typedef TT::Transform_t<TCTypes, TT::GetInsertIterator> TCInsert1;
    TCInsert0 transformInsert0;
    TCInsert1 transformInsert1;
    Holder< TCInsert0::types> holderInsert0 = transformInsert0 (tc0.m_fields);
    Holder< TCInsert1::types> holderInsert1 = transformInsert1 (tc1.m_fields);
    TT::Table_t< TCInsert0::types> insert0( holderInsert0 );
    TT::Table_t< TCInsert1::types> insert1( holderInsert1 );
    
    std::cout << "AAA0" << std::endl;

//     typedef TYPES_2(std::vector<int>::iterator, std::vector<int>::iterator) TCTypes;
//     typedef TT::Table_t<TCTypes>::table_type TC;
//     TC tc0;
//     TC tc1;
//     std::vector<int> tc0_0, tc0_1, tc1_0, tc1_1;
//     for (int i = 0; i < 20; ++i)
//     {
// 	tc0_0.push_back (1+i);
// 	tc0_1.push_back (2*i);
// 	tc1_0.push_back (5+i);
// 	tc1_1.push_back (i*i);
//     }
//     TT::Field<0>(tc0.m_fields) = tc0_0.begin();
//     TT::Field<1>(tc0.m_fields) = tc0_1.begin();
//     TT::Field<0>(tc1.m_fields) = tc1_0.begin();
//     TT::Field<1>(tc1.m_fields) = tc1_1.begin();

    for (int i = 0; i < 20; ++i)
    {
	std::cout << "insert f0 to insert0: " << i << std::endl;
	*TT::Field<0>(insert0.m_fields) = 1+i;
	std::cout << "insert f1 to insert0: " << i << std::endl;
//	FNC_COMPILERPRINT(insert0.m_fields);
	*TT::Field<1>(insert0.m_fields) = 2*i;
    }
    for (int i = 0; i < 10; ++i)
    {
	std::cout << "insert f0 to insert1: " << i << std::endl;
	*TT::Field<0>(insert1.m_fields) = 5+i;
	std::cout << "insert f1 to insert1: " << i << std::endl;
	*TT::Field<1>(insert1.m_fields) = i*i;
    }

    std::cout << "---" << std::endl;

    tc0.m_size = TT::Field<0>(tc0.m_fields).size();
    tc1.m_size = TT::Field<0>(tc1.m_fields).size();
    typedef TT::Transform_t<TCTypes, TT::GetIterator> TCIt0;
    typedef TT::Transform_t<TCTypes, TT::GetIterator> TCIt1;
    TCIt0 transformIt0;
    TCIt1 transformIt1;
    Holder< TCIt0::types> it0 = transformIt0 (tc0.m_fields);
    Holder< TCIt1::types> it1 = transformIt1 (tc1.m_fields);
    
//    TT::InnerJoin_t<TC,TC> innerJoin(tc0,tc1);
//    TT::MySelect select;
    TT::MySelectToTable select;
//    TT::MyWhere  where;
    MyOtherWhere  where;
//    innerJoin(select,where);

    std::cout << "========== " << std::endl;
//    InnerJoin (select,tc0)(select,where);

    // InnerJoin(
    // 	InnerJoin(tc0,tc1)(select,where),
    // 	tc0)
    // 	 (select,where);

    // myselecttotable is a table, .. print now its contents
    std::cout << "========== print the selected table ========== " << TT::Field<0>(select.m_fields).size() << std::endl;
    for (size_t i = 0, iEnd = TT::Field<0>(select.m_fields).size(); i < iEnd; ++i)
    {
	std::cout << "s<0> = " << TT::Field<0>(select.m_fields).at(i) 
		  << "  s<1> = " << TT::Field<1>(select.m_fields).at(i)
//		  << "  s<2> = " << TT::Field<2>(select.m_fields).at(i)
		  << std::endl;
    } 


    // TABLE_4(Mine,double,age,int,value,char,descriptor,int,other);
    // char(COMPILERPRINT<Mine>());

    Mine mine;
    Other other;

    for (int i = 0; i <20; ++i)
    {
	mine.age().push_back (11.1+i);
	mine.value().push_back ((i+1)*2);
	mine.number().push_back (-i);
//	++mine.m_size;
	mine.nextRow ();
    }
    for (int i = 0; i <20; ++i)
    {
	mine.age().push_back (400+i);
	mine.value().push_back (200+i);
	mine.number().push_back (-i);
//	++mine.m_size;
	mine.nextRow ();
    }

    for (int i = 0; i <20; ++i)
    {
	other.person().push_back (21.1*i);
	other.value().push_back ((i-1)*2);
//	++other.m_size;
	other.nextRow ();
    }


    TestSelect testSelect;

    std::cout << "========== inner join ========== " << std::endl;
//    InnerJoin (mine,other)(select,where);
    InnerJoin (mine,other)(testSelect,where);

//    TT::Field<0>(mine.m_fields).at(1) = 666;    

    std::cout << "========== print the selected table ========== " << TT::Field<0>(select.m_fields).size() << std::endl;
    for (size_t i = 0, iEnd = TT::Field<0>(testSelect.m_fields).size(); i < iEnd; ++i)
    {
	// double* ptr = TT::Field<2>(testSelect.m_fields).at(i);
	// std::cout << "pointer: ";
	// std::cout.operator<< ((const void*)ptr);
	// std::cout << "  ";
	// double value = *ptr;
	// std::cout.operator<< (value);
	// std::cout << "  value=";
	// std::cout.operator<< (*ptr);
	// std::cout << "  ";

	// ptr = &TT::Field<0>(mine.m_fields).at(i);
	// std::cout << "  pointer origin: ";
	// std::cout.operator<< ((const void*)ptr);
	// std::cout << "  ";
	// value = *ptr;
	// std::cout.operator<< (value);
	// std::cout << "  value=";
	// std::cout.operator<< (*ptr);
	// std::cout << "  ";

	std::cout << "s<0> = " << TT::Field<0>(testSelect.m_fields).at(i) 
		  << "  s<1> = " << TT::Field<1>(testSelect.m_fields).at(i)
		  << "  s<2> = " << *TT::Field<2>(testSelect.m_fields).at(i)
		  << "  &s<2> = " << TT::Field<2>(testSelect.m_fields).at(i)
		  << std::endl;
	
    } 
}



