#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cassert>


#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

// generic "begin" and "end" similar to those in C++11 (but not complete). 
#if (defined(GCC_VERSION) && GCC_VERSION >= 50200 && __cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900) || (defined(CLANG_VERSION) && CLANG_VERSION >= 3000)
    #pragma message( "C++11 version detected")

    #define HAS_RANDOM 1
    #define HAS_LAMBDAS 1
    #define HAS_AUTO 1
    #define HAS_IOTA 1


#else
    #pragma message( "C++98/03 version detected")

template <typename Container>
typename Container::iterator begin (Container& container)
{
    return container.begin ();
}

template <typename Container>
typename Container::iterator end (Container& container)
{
    return container.end ();
}

template <typename Container>
typename Container::const_iterator begin (const Container& container)
{
    return container.begin ();
}

template <typename Container>
typename Container::const_iterator end (const Container& container)
{
    return container.end ();
}

#endif




#ifdef HAS_RANDOM
    #include <random>
#else
#endif
    

template <typename T>
T randomValue (T minValue, T maxValue)
{
    #ifdef HAS_RANDOM
    static std::default_random_engine generator;
    static std::uniform_int_distribution<T> distribution(minValue ,maxValue);
    return distribution(generator);
    #else
    return rand()%(maxValue-minValue+1) + minValue;    
    #endif
}


#ifdef HAS_AUTO
auto uniform (auto minValue, auto maxValue)
{
    return
	[minValue, maxValue](auto& container)
            {
	       std::for_each (begin (container), end (container), [&](auto& v)
		       {
			   v = randomValue (minValue, maxValue);
		       });
	    };
}


void initialize (auto& values, auto from, auto to)
{
    std::for_each (begin (values), end (values), uniform (from, to));
}



void print (const auto& values)
{
    std::for_each (begin (values), end (values), [](const auto& row)
		   {
		       std::for_each (begin (row), end (row), [](const auto& v)
				      {
					  std::cout << " " << std::setw (3) << v;
				      });
		       std::cout << std::endl;
		   });
}

void print (const auto& name, const auto& values)
{
    std::cout << name << std::endl; std::copy (begin (values), end (values), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
}

#else
template <typename Container, typename T>
void initialize (Container& values, T from, T to)
{
    typedef typename Container::iterator column_iterator;
    typedef typename Container::value_type column_type;
    typedef typename column_type::iterator iterator;
    
    for (column_iterator itCol = values.begin (), itColEnd = values.end (); itCol != itColEnd; ++itCol)
    {
	column_type& column = *itCol;
	for (iterator it = begin (column), itEnd = end (column); it != itEnd; ++it)
	{
	    *it = randomValue (from, to);
	}
    }
}

template <typename Container>
void print (const Container& values)
{
    for (typename Container::const_iterator it = values.begin (), itEnd = values.end (); it != itEnd; ++it)
    {
	const typename Container::value_type& column = *it;
	for (typename Container::value_type::const_iterator itRow = column.begin (), itRowEnd = column.end (); itRow != itRowEnd; ++itRow)
	{
	    std::cout << " " << std::setw (3) << (*itRow);
	}
	std::cout << std::endl;
    }
}

template <typename String, typename Container>
void print (const String& name, const Container& values)
{
    std::cout << name << std::endl; std::copy (begin (values), end (values), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
}

#endif











#ifndef HAS_IOTA
template <class ForwardIterator, class T>
void impl_iota (ForwardIterator first, ForwardIterator last, T val)
{
    while (first!=last)
    {
        *first = val;
        ++first;
        ++val;
    }
}
#endif


#ifndef HAS_LAMBDAS
template <typename Container>
struct PermutationSortWeakOrdering
{
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::value_type column_type;
    typedef typename Container::value_type::value_type value_type;
    
    
    PermutationSortWeakOrdering (const Container& container)
	: m_container (container)
    {}

    bool operator () (size_t rowLhs, size_t rowRhs) const
    {
	for (const_iterator it = begin (m_container), itEnd = end (m_container); it != itEnd; ++it)
	{
	    const column_type& column = *it;
	    if (column.at (rowLhs) < column.at (rowRhs))
		return true;
	    else if (column.at (rowLhs) > column.at (rowRhs))
		return false;
	}
	return false;
    }
    
private:
    const Container& m_container;
};



template <typename Container>
struct PermutationNotEqual
{
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::value_type column_type;
    typedef typename Container::value_type::value_type value_type;
    
    
    PermutationNotEqual (const Container& container)
	: m_container (container)
    {}

    bool operator () (size_t rowLhs, size_t rowRhs) const
    {
	for (const_iterator it = begin (m_container), itEnd = end (m_container); it != itEnd; ++it)
	{
	    const column_type& column = *it;
	    if (column.at (rowLhs) != column.at (rowRhs))
		return false;
	}
	return true;
    }
    
private:
    const Container& m_container;
};


#endif


// find the sort permutation from the vector of keys
template <typename Container>
void sort_permutation (const Container& container, std::vector<size_t>& permutation)
{
    #ifdef HAS_LAMBDAS
    std::sort (permutation.begin (), permutation.end (),[&container](auto rowLhs, auto rowRhs)
    	       {
    		   for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
    		   {
    		       const auto& column = *it;
    		       if (column.at (rowLhs) < column.at (rowRhs))
    			   return true;
    		       else if (column.at (rowLhs) > column.at (rowRhs))
    			   return false;
    		   }
    		   return false;
    	       });

    #else
    std::sort (permutation.begin (), permutation.end (),
	       PermutationSortWeakOrdering<Container> (container));

    #endif
}


// uniquify the permutation
template <typename Container>
void uniquify_permutation (const Container& container, std::vector<size_t>& permutation)
{
    #ifdef HAS_LAMBDAS
    auto itEnd = std::unique (permutation.begin (), permutation.end (), [&container](auto rowLhs, auto rowRhs)
	       {
		   for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
		   {
		       const auto& column = *it;
		       if (column.at (rowLhs) != column.at (rowRhs))
			   return false;
		   }
		   return true;
	       });
    permutation.resize (std::distance (begin (permutation), itEnd));
    #else
    typename std::vector<size_t>::iterator itEnd = std::unique (permutation.begin (), permutation.end (), PermutationNotEqual<Container>(container));
    permutation.resize (std::distance (permutation.begin (), itEnd));
    #endif
}


template <typename Sortee, typename Permutation>
Sortee apply_permutation(const Sortee& vec, const Permutation& p)
{
    Sortee sorted_vec;
    sorted_vec.reserve (p.size ());
    #ifdef HAS_LAMBDAS
    std::transform (p.begin(), p.end(), std::inserter (sorted_vec, sorted_vec.end ()), [&](std::size_t i)
                    {
                        return vec[i];
                    });

    #else
    // --- alternative version for older compilers
    typename std::back_insert_iterator<Sortee> itSorted = std::back_inserter (sorted_vec);
    for (std::vector<size_t>::const_iterator itP = p.begin (), itPEnd = p.end (); itP != itPEnd; ++itP, ++itSorted)
    {
        *itSorted = vec[*itP];
    }
    #endif
    return sorted_vec;
}


#ifndef HAS_LAMBDAS
template <typename Permutation>
struct ApplyPermutation
{
    ApplyPermutation (const Permutation& permutation)
	: m_permutation (permutation)
    {}

    template <typename Column>
    Column operator () (const Column& column) const
    {
	Column sorted;
	sorted.reserve (m_permutation.size ());
	typename std::back_insert_iterator<Column> itSorted (std::back_inserter (sorted));
	for (std::vector<size_t>::const_iterator itP = m_permutation.begin (), itPEnd = m_permutation.end (); itP != itPEnd; ++itP, ++itSorted)
	{
	    *itSorted = column[*itP];
	}
    
	return sorted;
    }

private:
    const Permutation& m_permutation;
};
#endif

#ifndef HAS_LAMBDAS
template <typename Container, typename Permutation>
Container apply (const Container& values, const Permutation& permutation)
{
    Container result;
    std::transform (begin (values), end (values), std::inserter (result, result.end ()),
		    ApplyPermutation<Permutation>(permutation));
    return result;
}    
#else
template <typename Container>
Container apply (const Container& values, const auto& permutation)
{
    Container result;
    std::transform (begin (values), end (values), std::inserter (result, result.end ()), [&permutation](const auto& col)
		   {
		       return apply_permutation (col, permutation);
		   });
    return result;
}    
#endif




#ifndef HAS_LAMBDAS
template <typename Container>
struct MultiColumnDifferenceWeakOrdering
{
    typedef typename Container::const_iterator const_iterator;
    typedef typename Container::value_type column_type;
    typedef typename Container::value_type::value_type value_type;
    
    MultiColumnDifferenceWeakOrdering (const Container& contPlus, const Container& contMinus, size_t limitPlus)
	: m_contPlus (contPlus)
	, m_contMinus (contMinus)
	, m_limitPlus (limitPlus)
    {}


    bool operator ()(size_t rowA, size_t rowB) const
    {
	for (const_iterator
		 itPlus = begin (m_contPlus),
		 itPlusEnd = end (m_contPlus),
		 itMinus = begin (m_contMinus),
		 itMinusEnd = end (m_contMinus);
	     itPlus != itPlusEnd && itMinus != itMinusEnd; ++itPlus, ++itMinus)
	{
	    const column_type& columnPlus = *itPlus;
	    const column_type& columnMinus = *itMinus;
	    const value_type& valPlus (rowA < m_limitPlus ? columnPlus.at (rowA) : columnMinus.at (rowA - m_limitPlus));
	    const value_type& valMinus (rowB < m_limitPlus ? columnPlus.at (rowB) : columnMinus.at (rowB - m_limitPlus));
	    if (valPlus < valMinus)
		return true;
	    else if (valMinus < valPlus)
		return false;
	}
	return false;
    }
    
    
private:
    const Container& m_contPlus;
    const Container& m_contMinus;

    const size_t m_limitPlus;
};
#else
auto multiColumWeakOrdering (const auto& contPlus, const auto& contMinus, size_t limitPlus)
{
    std::cout << "mcwo : limitPlus = " << limitPlus << std::endl;
    return [&, limitPlus](auto rowA, auto rowB)
    {
	for (auto
		 itPlus = begin (contPlus),
		 itPlusEnd = end (contPlus),
		 itMinus = begin (contMinus),
		 itMinusEnd = end (contMinus);
	     itPlus != itPlusEnd && itMinus != itMinusEnd; ++itPlus, ++itMinus)
	{
	    const auto& columnPlus = *itPlus;
	    const auto& columnMinus = *itMinus;
	    auto valPlus (rowA < limitPlus ? columnPlus.at (rowA) : columnMinus.at (rowA - limitPlus));
	    auto valMinus (rowB < limitPlus ? columnPlus.at (rowB) : columnMinus.at (rowB - limitPlus));
	    if (valPlus < valMinus)
		return true;
	    else if (valMinus < valPlus)
		return false;
	}
	return false;
    };
}
#endif



// uniquify the permutation
template <typename Container>
Container difference_permutation (const Container& contPlus, const Container& contMinus)
{
    // distinguish  between the indices (permutations) for contPlus from those for contMinus
    // by their values. From 0 to limitPlus --> contPlus, from limitPlus+1 to end for contMinus

    if (contPlus.at (0).size () == 0)
	return Container ();
    
    if (contMinus.at (0).size () == 0)
	return contPlus;
    
    std::vector<size_t> permPlus (contPlus.at (0).size (), 0);
    #ifdef HAS_IOTA
    std::iota (begin (permPlus), end (permPlus), 0);
    #else
    impl_iota (begin (permPlus), end (permPlus), 0);
    #endif
    std::vector<size_t> permMinus (contMinus.at (0).size (), 0);

    size_t limitPlus (permPlus.back () + 1);
    #ifdef HAS_IOTA
    std::iota (begin (permMinus), end (permMinus), limitPlus);
    #else
    impl_iota (begin (permMinus), end (permMinus), limitPlus);
    #endif
    std::vector<size_t> permTarget;

    std::set_difference	(begin (permPlus), end (permPlus),
			 begin (permMinus), end (permMinus),
			 std::back_inserter (permTarget),
			 #ifndef HAS_LAMBDAS
			 MultiColumnDifferenceWeakOrdering<Container> (contPlus, contMinus, limitPlus)
			 #else
			 multiColumWeakOrdering (contPlus, contMinus, limitPlus)
			 #endif
	);

    return apply (contPlus, permTarget);
}




template <typename Container>
void uniquify (Container& cont)
{
    std::vector<size_t> permutation (cont.at(0).size (), 0);
#ifdef HAS_IOTA
    std::iota (begin (permutation), end (permutation), size_t(0));
#else
    //                impl_iota (begin (permutation), end (permutation), size_t(0));
    impl_iota (permutation.begin (), permutation.end (), size_t(0));
#endif

    sort_permutation (cont, permutation);
    uniquify_permutation (cont, permutation);
    cont = apply (cont, permutation);
}


#ifdef HAS_LAMBDAS
template <typename Container>
Container& operator+= (Container& cont, const Container& contAdd)
{
    assert (cont.size () == contAdd.size ());
    auto itAdd = begin (contAdd);
    std::for_each (begin (cont), end (cont), [&itAdd](auto& column)
		   {
		       column.insert (end (column), begin (*itAdd), end (*itAdd));
		       ++itAdd;
		   });
    uniquify (cont);
    return cont;
}
#else
template <typename Container>
Container& operator+= (Container& cont, const Container& contAdd)
{
    assert (cont.size () == contAdd.size ());
    typename Container::const_iterator itAdd = begin (contAdd);
    for (typename Container::iterator itC = begin (cont), itCEnd = end (cont); itC != itCEnd; ++itC)
    {
	typename Container::value_type& column = *itC; 
	column.insert (end (column), begin (*itAdd), end (*itAdd));
	++itAdd;
    }
    uniquify (cont);
    return cont;
}
#endif


template <typename Container>
Container operator- (const Container& contPlus, const Container& contMinus)
{
    Container diff (difference_permutation (contPlus, contMinus));
    return diff;
}

template <typename Container>
Container& operator-= (Container& contPlus, const Container& contMinus)
{
    contPlus = difference_permutation (contPlus, contMinus);
    return contPlus;
}


template <typename Container>
Container operator+ (const Container& cont, const Container& contAdd)
{
    Container contSum (cont);
    return contSum += contAdd;
}




int main()
{
    const int dimensions = 1;
    const int length = 10;
    const int lengthMinus = 5;

    const int minVal (0);
    const int maxVal (15);

    typedef std::vector<std::vector<int> > container_type;
    container_type values (dimensions, std::vector<int> (length,0));
    initialize (values, minVal, maxVal);

    std::vector<std::vector<int> > valuesMinus (dimensions, std::vector<int> (lengthMinus,0));
    initialize (valuesMinus, minVal, maxVal);

    
    std::vector<size_t> permutation (length);
    #ifdef HAS_IOTA
    std::iota (begin (permutation), end (permutation), 0);
    #else
    impl_iota (begin (permutation), end (permutation), 0);
    #endif
//    print ("iota", permutation);

    sort_permutation (values, permutation);
//    print ("sorted", permutation);
    uniquify_permutation (values, permutation);
    
//    print ("uniquified", permutation);

    std::cout << "--- input ---" << std::endl;
    print (values);
    values = apply (values, permutation);
    
    
	
    std::cout << "--- sorted ---" << std::endl;
    print (values);


    uniquify (valuesMinus);

    
    std::cout << "--- sorted values to diff ---" << std::endl;
    print (valuesMinus);


    std::cout << "--- diffing ---" << std::endl;
    container_type diff = values - valuesMinus;

//    std::cout << "diff result" << std::endl; std::copy (begin (diff), end (permDiff), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
    std::cout << "diff result" << std::endl;
    print (diff);

    std::cout << "--- diffing Minus---" << std::endl;
    container_type diffMinus = valuesMinus -  values;
    print (diffMinus);

    values += diffMinus;
    std::cout << "adding difference between valuesMinus and values" << std::endl;
    print (values);

}

