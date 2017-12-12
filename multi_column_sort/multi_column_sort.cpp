#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>


#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)

// generic "begin" and "end" similar to those in C++11 (but not complete). 
#if (defined(GCC_VERSION) && GCC_VERSION >= 50200 && __cplusplus >= 201103L) || (defined(_MSC_VER) && _MSC_VER >= 1900) || (defined(CLANG_VERSION) && CLANG_VERSION >= 3000)
    #pragma message( "C++11 version detected")

    #define HAS_RANDOM 1
    #define HAS_LAMBADS 1
    #define HAS_AUTO 1


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
    
    // std::transform (p.begin(), p.end(), std::inserter (sorted_vec, sorted_vec.end ()), [&](std::size_t i)
    //                 {
    //                     return vec[i];
    //                 });

    // --- alternative version for older compilers
    typename std::back_insert_iterator<Sortee> itSorted = std::back_inserter (sorted_vec);
    for (std::vector<size_t>::const_iterator itP = p.begin (), itPEnd = p.end (); itP != itPEnd; ++itP, ++itSorted)
    {
        *itSorted = vec[*itP];
    }
    
    return sorted_vec;
}


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


template <typename Container, typename Permutation>
Container apply (const Container& values, const Permutation& permutation)
{
    Container result;
    std::transform (begin (values), end (values), std::inserter (result, result.end ()),
		    ApplyPermutation<Permutation>(permutation));
    return result;
}    

// template <typename Container>
// Container apply (const Container& values, const auto& permutation)
// {
//     Container result;
//     std::transform (begin (values), end (values), std::inserter (result, result.end ()), [&permutation](const auto& col)
// 		   {
// 		       return apply_permutation (col, permutation);
// 		   });
//     return result;
// }    






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




// uniquify the permutation
template <typename Container>
Container difference_permutation (const Container& contPlus, const Container& contMinus)
{
    // distinguish  between the indices (permutations) for contPlus from those for contMinus
    // by their values. From 0 to limitPlus --> contPlus, from limitPlus+1 to end for contMinus
    
    std::vector<size_t> permPlus (contPlus.at (0).size (), 0);
    impl_iota (begin (permPlus), end (permPlus), 0);
    std::vector<size_t> permMinus (contMinus.at (0).size (), 0);

    size_t limitPlus (permPlus.back () + 1);
    impl_iota (begin (permMinus), end (permMinus), limitPlus);
    std::vector<size_t> permTarget;

    std::set_difference (begin (permPlus), end (permPlus),
			 begin (permMinus), end (permMinus),
			 std::back_inserter (permTarget),
			 MultiColumnDifferenceWeakOrdering<Container> (contPlus, contMinus, limitPlus)
			 // [&](auto rowA, auto rowB)
			 // {
			 //     for (auto
			 // 	      itPlus = begin (contPlus),
			 // 	      itPlusEnd = end (contPlus),
			 // 	      itMinus = begin (contMinus),
			 // 	      itMinusEnd = end (contMinus);
			 // 	  itPlus != itPlusEnd && itMinus != itMinusEnd; ++itPlus, ++itMinus)
			 //     {
			 // 	 const auto& columnPlus = *itPlus;
			 // 	 const auto& columnMinus = *itMinus;
			 // 	 auto valPlus (rowA < limitPlus ? columnPlus.at (rowA) : columnMinus.at (rowA - limitPlus));
			 // 	 auto valMinus (rowB < limitPlus ? columnPlus.at (rowB) : columnMinus.at (rowB - limitPlus));
			 // 	 if (valPlus < valMinus)
			 // 	     return true;
			 // 	 else if (valMinus < valPlus)
			 // 	     return false;
			 //     }
			 //     return false;
			 // }
	);

    return apply (contPlus, permTarget);
}





int main()
{
    const int dimensions = 4;
    const int length = 20;
    const int lengthMinus = 15;

    const int minVal (0);
    const int maxVal (3);

    typedef std::vector<std::vector<int> > container_type;
    container_type values (dimensions, std::vector<int> (length,0));
    initialize (values, minVal, maxVal);

    std::vector<std::vector<int> > valuesMinus (dimensions, std::vector<int> (lengthMinus,0));
    initialize (valuesMinus, minVal, maxVal);

    
    std::vector<size_t> permutation (length);
    impl_iota (begin (permutation), end (permutation), 0);
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


    std::vector<size_t> permutationMinus (lengthMinus);
    impl_iota (begin (permutationMinus), end (permutationMinus), 0);
    sort_permutation (valuesMinus, permutationMinus);
    uniquify_permutation (valuesMinus, permutationMinus);
    valuesMinus = apply (valuesMinus, permutationMinus);

    
    std::cout << "--- sorted values to diff ---" << std::endl;
    print (valuesMinus);

    std::cout << "--- diffing ---" << std::endl;
    container_type diff = difference_permutation (values, valuesMinus);

//    std::cout << "diff result" << std::endl; std::copy (begin (diff), end (permDiff), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
    std::cout << "diff result" << std::endl;
    print (diff);
}

