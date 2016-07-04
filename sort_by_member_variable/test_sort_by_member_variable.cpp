#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>


template <typename R, typename T, R (T::*Fnc)()>
class mem_fun_wrapper_t
{
public:
    typedef R return_type;
    typedef T class_type;
    inline return_type operator()(const class_type&  obj) const
    {
	return (obj.*Fnc)();
    }
};




template <typename R, typename T, R (T::*Fnc)() const>
class mem_fun_wrapper_const_t
{
public:
    typedef R return_type;
    typedef T class_type;
    inline return_type operator()(const class_type&  obj) const 
    {
	return (obj.*Fnc)();
    }
};



template <typename R, typename T, R const T::*member_variable, typename IR, R (IR::*Fnc)()>
class mem_var_wrapper_t {
public:
    typedef R return_type;
    typedef T class_type;
    inline return_type operator()(const class_type&  obj) const
    {
	return obj.*member_variable.*Fnc();
    }
};




class A
{
public:
    A (int a, int b) : m_valueA(a), m_valueB(b) {}

    int m_valueA;
    int m_valueB;
    
    int getA ()  { return m_valueA; }
    int getB () const { return m_valueB; }

    template <typename OutStream>
    A& operator<<(OutStream& stream)  { stream << "(" << m_valueA << "|" << m_valueB << ")"; return *this; } 

    friend std::ostream& operator<<(std::ostream& os, const A& a)
    { 
	os << "(" << a.m_valueA << "|" << a.m_valueB << ")"; 
	return os;
    } 


};


namespace X 
{
class A
{
public:
    A (int a, int b) : m_valueA(a), m_valueB(b) {}

    int m_valueA;
    int m_valueB;
    
    int getA ()  { return m_valueA; }
    int getB () const { return m_valueB; }

    template <typename OutStream>
    A& operator<<(OutStream& stream)  { stream << "(" << m_valueA << "|" << m_valueB << ")"; return *this; } 

    friend std::ostream& operator<<(std::ostream& os, const A& a)
    { 
	os << "(" << a.m_valueA << "|" << a.m_valueB << ")"; 
	return os;
    } 


};
}



template <bool ReturnValue = false>
class Return
{
public:
    template <typename T>
    bool operator()(const T&, const T&) const { return ReturnValue; }
};


template <typename return_type, typename T, return_type const T::*member_variable, template <typename> class SortPolicy, typename SubSort = Return<true> >
class SortByMemberVariable {
public:
    bool operator()(const T& lhs, const T& rhs) const 
    { 
	if (lhs.*member_variable == rhs.*member_variable)
	    return SubSort()(lhs,rhs);
	return SortPolicy<return_type>()(lhs.*member_variable, rhs.*member_variable); 
    }
};



// template <typename return_type, typename T, return_type (T::*member_function)(), template <typename> class SortPolicy, typename SubSort = Return<false> >
// class SortByMemberFunction {
// public:
//     bool operator()( const T& lhs, const T& rhs) const 
//     { 
// 	if ((lhs.*member_function)() == (rhs.*member_function)())
// 	    return SubSort()(lhs,rhs);
// 	return SortPolicy<return_type>()((lhs.*member_function)(), (rhs.*member_function)()); 
//     }
// };

template <typename return_type, typename T, return_type (T::*member_function)() const, template <typename> class SortPolicy, typename SubSort = Return<false> >
class SortByMemberFunctionConst {
public:
    bool operator()(const T& lhs, const T& rhs) const 
    { 
	if ((lhs.*member_function)() == (rhs.*member_function)())
	    return SubSort()(lhs,rhs);
	return SortPolicy<return_type>()((lhs.*member_function)(), (rhs.*member_function)()); 
    }
};

template <typename Fnc, template <typename> class SortPolicy, typename SubSort = Return<false> >
class SortByFunctor {
public:
    bool operator()(const typename Fnc::class_type& lhs, const typename Fnc::class_type& rhs) const
    { 
	Fnc functor;
	if (functor(lhs) == functor(rhs))
	    return SubSort()(lhs,rhs);
	return SortPolicy<typename Fnc::return_type>()(functor(lhs), functor(rhs)); 
    }
};






int main()
{
    std::vector<A> vec;

    vec.push_back (A(1,3));
    vec.push_back (A(8,2));
    vec.push_back (A(9,9));
    vec.push_back (A(1,2));
    vec.push_back (A(14,3));
    vec.push_back (A(6,3));
    vec.push_back (A(1,11));
    vec.push_back (A(1,2));
    vec.push_back (A(7,8));

    

    std::sort (vec.begin(),vec.end(),SortByMemberVariable<int,A,&A::m_valueA,std::less>());
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;

    std::sort (vec.begin(),vec.end(),SortByMemberVariable<int,A,&A::m_valueA,std::greater>());
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;

    std::sort (vec.begin(),vec.end(),SortByMemberVariable<int,A,&A::m_valueB,std::greater>());
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;

    std::sort (vec.begin(),vec.end(),SortByMemberVariable<int,A,&A::m_valueB,std::less>());
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;

    std::sort (vec.begin(),vec.end(),
	       SortByMemberVariable<int,
			    A,
			    &A::m_valueB,
			    std::less,SortByMemberVariable<
				int,A,&A::m_valueA, std::greater
				> 
			    > ());  
	       std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;
		 
    // 		 std::sort (vec.begin(),vec.end(),std::mem_fun_ref(&A::getA));
    // std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    // std::cout << std::endl;

    std::sort (vec.begin(),vec.end(),SortByMemberFunctionConst<int,A,&A::getB,std::less>());
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    std::cout << std::endl;

    // std::sort (vec.begin(),vec.end(),SortByMemberFunction<int,A,&A::getA,std::less>());
    // std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    // std::cout << std::endl;


    int (A::*member_function_ptr)() const = &A::getB;
    A a(5,6);
    int value = (a.*member_function_ptr)();
    


		 std::vector<X::A> vec2;

    vec2.push_back (X::A(1,3));
    vec2.push_back (X::A(8,2));
    vec2.push_back (X::A(9,9));
    vec2.push_back (X::A(1,2));
    vec2.push_back (X::A(14,3));
    vec2.push_back (X::A(6,3));
    vec2.push_back (X::A(1,11));
    vec2.push_back (X::A(1,2));
    vec2.push_back (X::A(7,8));


    std::sort (vec2.begin(),vec2.end(),SortByFunctor<mem_fun_wrapper_const_t<int,X::A,&X::A::getB>,std::less>());
    std::copy (vec2.begin(), vec2.end(), std::ostream_iterator<typename X::A>(std::cout,":"));
    std::cout << std::endl;

    // std::sort (vec.begin(),vec.end(),SortByFunctor<mem_fun_wrapper_t<int,A,&A::getA>,std::less>());
    // std::copy (vec.begin(), vec.end(), std::ostream_iterator<A>(std::cout,":"));
    // std::cout << std::endl;


    std::cout << std::endl;

    std::vector<std::pair<int,X::A> > vec3;

    // typedef mem_fun_wrapper_const_t<int,X::A,&X::A::getB> AToGetB;
    // typedef mem_var_wrapper_t<int,std::pair<int,X::A>,&std::pair<int,X::A>::first, X::A, &X::A::getB> PairToA;

    // typedef SortByFunctor<PairToA, PairToA::return_type, PairToA::class_type, PairToA
    // 	mem_fun_wrapper_const_t<
    // 	    int,
    // 	    std::pair<int,X::A>::second_type,
    // 	    &std::pair<int,X::A>::second_type::getB>,std::less, Return<false> > SortByGetB;

    // std::sort (vec3.begin(), vec3.end(), SortByGetB());
    // std::copy (vec3.begin(), vec3.end(), std::ostream_iterator<std::pair<int,typename X::A> >(std::cout,":"));
    // std::cout << std::endl;
}



