#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <random>





template <typename T>
T randomValue (T minValue, T maxValue)
{
    static std::default_random_engine generator;
    static std::uniform_int_distribution<T> distribution(minValue ,maxValue);
    return distribution(generator);
}


template <typename Container, typename T>
auto uniform (Container& container, T minValue, T maxValue)
{
    return
	[&](auto& container)
            {
	       std::for_each (begin (container), end (container), [&](auto& v)
		       {
			   v = randomValue (minValue, maxValue);
		       });
	    };
}


void initialize (auto& values, auto from, auto to)
{
    std::for_each (begin (values), end (values), uniform (values, from, to));
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






// find the sort permutation from the vector of keys
template <typename Container>
void sort_permutation (const Container& container, std::vector<size_t>& permutation)
{
    std::sort (permutation.begin (), permutation.end (), [&container](auto rowLhs, auto rowRhs)
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
}


// uniquify the permutation
template <typename Container>
void uniquify_permutation (const Container& container, std::vector<size_t>& permutation)
{
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
}



// uniquify the permutation
template <typename Container>
void difference_permutation (const Container& contA,
			     std::vector<size_t>& permA,
			     const Container& contB,
			     std::vector<size_t>& permb)
{
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
}



template <typename Sortee, typename Permutation>
Sortee apply_permutation(const Sortee& vec, const Permutation& p)
{
    Sortee sorted_vec (p.size());
    
    std::transform (p.begin(), p.end(), sorted_vec.begin(), [&](std::size_t i)
                    {
                        return vec[i];
                    });

    // --- alternative version for older compilers
    // std::vector<T>::iterator itSorted = sorted_vec.begin ();
    // for (std::vector<size_t>::iterator itP = p.begin (), itPEnd = p.end (); itP != itPEnd; ++itP, ++itSorted)
    // {
    //     *itSorted = vec[*itP];
    // }
    
    return sorted_vec;
}





int main()
{
    const int dimensions = 2;
    const int length = 20;
    std::vector<std::vector<int> > values (dimensions, std::vector<int> (length,11));

    initialize (values, 0, 3);

    std::vector<size_t> permutation (length);
    impl_iota (begin (permutation), end (permutation), 0);
    std::cout << "iota" << std::endl; std::copy (begin (permutation), end (permutation), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;

    sort_permutation (values, permutation);
    std::cout << "sorted" << std::endl; std::copy (begin (permutation), end (permutation), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
    uniquify_permutation (values, permutation);
    
    std::cout << "uniquified" << std::endl;
    std::copy (begin (permutation), end (permutation), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "--- input ---" << std::endl;
    print (values);
    std::for_each (begin (values), end (values), [&permutation](auto& col)
		   {
		       col = apply_permutation (col, permutation);
		   });
    
    
	
    std::cout << "--- sorted ---" << std::endl;
    print (values);
}

