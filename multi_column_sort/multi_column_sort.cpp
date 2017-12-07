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


template <typename Sortee, typename Permutation>
Sortee apply_permutation(const Sortee& vec, const Permutation& p)
{
    Sortee sorted_vec;
    sorted_vec.reserve (p.size ());
    
    std::transform (p.begin(), p.end(), std::inserter (sorted_vec, sorted_vec.end ()), [&](std::size_t i)
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



void print (const auto& name, const auto& values)
{
    std::cout << name << std::endl; std::copy (begin (values), end (values), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
}


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
			 [&](auto rowA, auto rowB)
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
			 });

    return apply (contPlus, permTarget);
}





int main()
{
    const int dimensions = 4;
    const int length = 20;
    const int lengthMinus = 15;

    const int minVal (0);
    const int maxVal (3);
    
    std::vector<std::vector<int> > values (dimensions, std::vector<int> (length,0));
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
    auto diff = difference_permutation (values, valuesMinus);

//    std::cout << "diff result" << std::endl; std::copy (begin (diff), end (permDiff), std::ostream_iterator<int>(std::cout, " ")); std::cout << std::endl;
    std::cout << "diff result" << std::endl;
    print (diff);
}

