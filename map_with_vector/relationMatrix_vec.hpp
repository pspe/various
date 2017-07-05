#pragma once


#include <vector>
#include <iterator>
#include <functional>
#include <algorithm>

#include "iks.hpp"



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




template <typename Comparison, class InputIterator1, class InputIterator2, class OutputIterator>
void set_intersection_retain_duplicates_of_first (InputIterator1 first1, InputIterator1 last1,
                                                  InputIterator2 first2, InputIterator2 last2,
                                                  OutputIterator keepIndex)
{
    size_t idx (0);
    while (first1!=last1 && first2!=last2)
    {
        if (Comparison () (*first1,*first2))
        {
            ++first1;
            ++idx;
        }
        else if (Comparison () (*first2,*first1))
        {
            ++first2;
        }
        else
        {
            const auto& val = *first1;
            auto first1Upper = std::upper_bound (first1, last1, val, Comparison ());
            while (first1 != first1Upper)
            {
                *keepIndex = idx;
                ++keepIndex;
                ++first1;
                ++idx;
            }
            
            ++first2;
        }
    }
}



template <typename CONTAINER, typename COMPARE>
struct CompareFunctor
{
    CompareFunctor (const CONTAINER& _container)
        : compare ()
        , container (_container)
          
    {
    }
    
    template <typename T>
    bool operator() (const T& lhs, const T& rhs) const
    {
        return compare (container[lhs], container[rhs]);
    }

    COMPARE compare;
    const CONTAINER& container;
};



// find the sort permutation from the vector of keys
template <typename T, typename COMPARE>
std::vector<size_t> sort_permutation (const std::vector<T>& vec)
{
    std::vector<size_t> permutation (vec.size ());
    impl_iota (permutation.begin (), permutation.end (), 0);
    std::sort (permutation.begin (), permutation.end (), CompareFunctor<const std::vector<T>, COMPARE> (vec));
    return permutation;
}

// find the sort permutation from the vector of keys
template <typename T, typename COMPARE>
void sort_permutation_2 (const std::vector<T>& vec, std::vector<size_t>& permutation)
{
    // std::vector<size_t> permutation (vec.size ());
    // impl_iota (permutation.begin (), permutation.end (), 0);
    std::sort (permutation.begin (), permutation.end (), CompareFunctor<const std::vector<T>, COMPARE> (vec));
}




// find key-value duplicates
template <typename key_comparison, typename value_comparison, typename key_container, typename value_container>
void remove_key_value_duplicates (key_container& keys, value_container& values)
{
    if (begin (keys) == end (keys) || keys.size () != values.size ())
        return;

    std::vector<char> mark (keys.size (), char (1));
    auto firstKey = begin (keys);
    auto lastKey = end (keys);
    auto firstValue = begin (values);
    
    auto itMark = begin (mark);
    auto itKey = firstKey;
    auto itValue = firstValue;
    while (itKey != lastKey)
    {
        auto itKeyUpper = std::upper_bound (itKey, lastKey, *itKey, key_comparison ());
        auto distRange = itKeyUpper - itKey; // std::distance (itKey, itKeyUpper);
        // if (distRange > 1)
        {
            auto itValueLocalEnd = itValue + distRange;
            std::sort (itValue, itValueLocalEnd, value_comparison ());
            auto itValueLocalNewEnd = std::unique (itValue, itValueLocalEnd);
            auto distUnique = itValueLocalNewEnd - itValue; //std::distance (itValue, itValueLocalNewEnd);
            std::fill (itMark + distUnique, itMark + distRange, char (0));
            itValue = itValueLocalEnd;
        }
        // else
        //     itValue += distRange;
        itMark += distRange;
        itKey = itKeyUpper;
    }

    itKey = firstKey;
    itValue = firstValue;
    auto itKeyResult = firstKey;
    auto itValueResult = firstValue;
    itMark = begin (mark);
    auto itMarkEnd = end (mark);
    while (itMark != itMarkEnd)
    {
        if (!*itMark)
        {
            ++itMark;
            ++itKey;
            ++itValue;
            continue;
        }
        (*itKeyResult) = std::move (*itKey);
        (*itValueResult) = std::move (*itValue);
        ++itKey;
        ++itValue;
        ++itKeyResult;
        ++itValueResult;
        ++itMark;
    }

    keys.erase (itKeyResult, end (keys));
    values.erase (itValueResult, end (values));
}


template <typename CONTAINER>
void remove_key_value_duplicates_2 (CONTAINER& keys, CONTAINER& values)
{
    if (begin (keys) == end (keys) || keys.size () != values.size ())
        return;

    auto itKeyEnd = end (keys);

    auto itKey = begin (keys);
    auto itValue = begin (values);

    auto itResultKey = itKey;
    auto itResultValue = itValue;
    
    while (++itKey != itKeyEnd)
    {
        ++itValue;

        if (*itResultKey == *itKey && *itResultValue == *itValue)
            continue;

        *(++itResultKey) == *itKey;
        *(++itResultValue) == *itValue;
    }
    
    keys.erase (itResultKey, end (keys));
    values.erase (itResultValue, end (values));
}



template <typename T>
std::vector<T> apply_permutation(const std::vector<T>& vec, const std::vector<std::size_t>& p)
{
    std::vector<T> sorted_vec (p.size());
    
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


template <typename T>
void apply_permutation(std::vector<T>& vec0, std::vector<T>& vec1, const std::vector<std::size_t>& p)
{
    std::vector<T> sorted_vec0;
    std::vector<T> sorted_vec1;
    sorted_vec0.reserve (p.size ());
    sorted_vec1.reserve (p.size ());
    for (auto idx : p)
    {
        sorted_vec0.push_back (vec0[idx]);
        sorted_vec1.push_back (vec1[idx]);
    }
    vec0.swap (sorted_vec0);
    vec1.swap (sorted_vec1);
}




template <typename T>
void apply_permutation_in_place (std::vector<T>& vec, const std::vector<std::size_t>& p)
{
    std::vector<bool> done(p.size());
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}



enum class SIDE { LEFT, RIGHT };




// template <typename left_type, typename right_type>
template <typename left_type, typename right_type, typename left_comparison_type = std::less<left_type>, typename right_comparison_type = std::less<right_type> >
class RelationMatrix2 
{
private:

    
public:


    RelationMatrix2 ()
        : m_orientation (SIDE::LEFT)
        , m_isDirty (false)
        , m_left ()
        , m_right ()
    {}

    RelationMatrix2 (const RelationMatrix2& other)
        : m_orientation (other.m_orientation)
        , m_isDirty (other.m_isDirty)
        , m_left (other.m_left)
        , m_right (other.m_right)
    {}

    RelationMatrix2& operator= (const RelationMatrix2& other)
    {
        m_isDirty = other.m_isDirty;
        m_left = other.m_left;
        m_right = other.m_right;
        m_orientation = other.m_orientation;
    }
        

    void clear ()
    {
        *this = RelationMatrix2 ();
    }

    size_t size () const
    {
        cleanUp (SIDE::LEFT);
        return m_left.size ();
    }

    
    size_t sizeLeft () const
    {
        return size (SIDE::LEFT);
    }

    size_t sizeRight () const
    {
        return size (SIDE::RIGHT);
    }

    void addRelation (left_type left, right_type right)
    {
        m_left. push_back (left );
        m_right.push_back (right);
        m_isDirty = true;
    }


    
    
    // template <typename CONTAINER>
    // void cleanUp (CONTAINER& keys, CONTAINER& values) const
    // {
    //     typedef typename CONTAINER::value_type value_type;
    //     // get the sort permutation from the keys
    //     std::vector<std::size_t> permutation = sort_permutation<value_type, std::less<value_type> >  (keys);

    //     // apply the sort permutation on keys and values
    //     keys   = apply_permutation (keys,   permutation);
    //     values = apply_permutation (values, permutation);
    //     // apply_permutation_in_place (keys,   permutation);
    //     // apply_permutation_in_place (values, permutation);

    //     // check for key-value duplicates
    //     remove_key_value_duplicates (keys, values);
    // }

    template <typename key_comparison, typename value_comparison, typename left_container, typename right_container>
    void cleanUp (left_container& keys, right_container& values) const
    {
        // CALLGRIND_START_INSTRUMENTATION;
        // CALLGRIND_TOGGLE_COLLECT;

	// typedef typename CONTAINER::value_type value_type;


        // get the sort permutation from the keys
        // std::vector<size_t> permutation (keys.size ());
        // impl_iota (permutation.begin (), permutation.end (), 0);

        // std::vector<std::size_t> permutation = sort_permutation<value_type, std::less<value_type> >  (values);
        // sort_permutation_2<value_type, std::less<value_type> >  (keys, permutation);
        // sort_permutation_2<value_type, std::less<value_type> >  (values, permutation);
        
        // keys   = apply_permutation (keys,   permutation);
        // values = apply_permutation (values, permutation);
        // apply_permutation_in_place (keys,   permutation);
        // apply_permutation_in_place (values, permutation);

        // permutation = sort_permutation<value_type, std::less<value_type> >  (keys);
	typedef typename left_container::value_type sort_by_type;
	std::vector<size_t> permutation = sort_permutation<sort_by_type, key_comparison>  (keys);

        // apply the sort permutation on keys and values
        keys   = apply_permutation (keys,   permutation);
        values = apply_permutation (values, permutation);

        // apply_permutation (keys, values, permutation);
        
        // apply_permutation_in_place (keys,   permutation);
        // apply_permutation_in_place (values, permutation);

        // check for key-value duplicates
        // remove_key_value_duplicates_2 (keys, values);
        remove_key_value_duplicates<key_comparison, value_comparison> (keys, values);

        // CALLGRIND_TOGGLE_COLLECT;
        // CALLGRIND_STOP_INSTRUMENTATION;

    }

    
    void cleanUp (SIDE eSide) const
    {
        if (eSide == SIDE::LEFT)
        {
            if (m_orientation == SIDE::LEFT && !m_isDirty)
                return;
            cleanUp<left_comparison_type, right_comparison_type> (m_left, m_right);
            m_isDirty = false;
            m_orientation = SIDE::LEFT;
            return;
        }
        if (m_orientation == SIDE::RIGHT && !m_isDirty)
            return;
        cleanUp<right_comparison_type, left_comparison_type> (m_right, m_left);
        m_isDirty = false;
        m_orientation = SIDE::RIGHT;
    }
    

    template <typename Comparison, typename left_container, typename right_container, typename Iterator>
    void keepFromSide (left_container& left,
		       right_container& right,
		       Iterator first,
		       Iterator last)
    {
        std::vector<size_t> keepIndex;
	size_t length = std::distance (first, last);
        keepIndex.reserve (length);
        set_intersection_retain_duplicates_of_first<Comparison> (
	    begin (left),
	    end (left),
	    first,
	    last,
	    std::back_inserter (keepIndex));
        left = apply_permutation (left, keepIndex);
        right = apply_permutation (right, keepIndex);
    }
  
    
    template <SIDE eSide, typename Iterator>
    void keepFromSide (Iterator first, Iterator last)
    {
        assert (std::is_sorted (first, last));
        cleanUp (eSide);
	if (eSide == SIDE::LEFT)
	    keepFromSide<left_comparison_type> (m_left, m_right, first, last);
	else if (eSide == SIDE::RIGHT)
	    keepFromSide<right_comparison_type> (m_right, m_left, first, last);
        m_orientation = eSide;
    }

    
    template <typename Iterator>
    void keepFromLeft (Iterator first, Iterator last)
    {
        assert (std::is_sorted (first, last, left_comparison_type ()));
        cleanUp (SIDE::LEFT);

        std::vector<size_t> keepIndex;
	size_t length = std::distance (first, last);
        keepIndex.reserve (length);
        set_intersection_retain_duplicates_of_first<left_comparison_type> (
	    begin (m_left),
	    end (m_left),
	    first,
	    last,
	    std::back_inserter (keepIndex));
        m_left = apply_permutation (m_left, keepIndex);
        m_right = apply_permutation (m_right, keepIndex);
    }

    template <typename Iterator>
    void keepFromRight (Iterator first, Iterator last)
    {
        assert (std::is_sorted (first, last, right_comparison_type ()));
        cleanUp (SIDE::RIGHT);

        std::vector<size_t> keepIndex;
	size_t length = std::distance (first, last);
        keepIndex.reserve (length);
        set_intersection_retain_duplicates_of_first<right_comparison_type> (
	    begin (m_right),
	    end (m_right),
	    first,
	    last,
	    std::back_inserter (keepIndex));
        m_right = apply_permutation (m_right, keepIndex);
        m_left = apply_permutation (m_left, keepIndex);
    }

    template <typename left_index_type, typename right_index_type, typename WriteIterator>
    void getLeft (const left_index_type* leftIndex, const right_index_type* rightIndex, WriteIterator writeIterator) const
    {
        if (!leftIndex && !rightIndex)
        {
            cleanUp (SIDE::LEFT);
	    std::unique_copy (std::begin (m_left), std::end (m_left), writeIterator);
            return;
        }

        auto sub (*this);
	if (rightIndex)
	    sub.keepFromRight (std::begin (*rightIndex), std::end (*rightIndex));
	if (leftIndex)
	    sub.keepFromLeft  (std::begin (*leftIndex),  std::end (*leftIndex));
	if (sub.m_orientation != SIDE::LEFT)
	    sub.cleanUp (SIDE::LEFT);
	std::unique_copy (std::begin (sub.m_left), std::end (sub.m_left), writeIterator);
    }

    template <typename left_index_type, typename right_index_type, typename WriteIterator>
    void getRight (const left_index_type* leftIndex, const right_index_type* rightIndex, WriteIterator writeIterator) const
    {
        if (!leftIndex && !rightIndex)
        {
            cleanUp (SIDE::RIGHT);
	    std::unique_copy (std::begin (m_right), std::end (m_right), writeIterator);
            return;
        }

        auto sub (*this);
        if (leftIndex)
            sub.keepFromLeft  (std::begin (*leftIndex),  std::end (*leftIndex));
        if (rightIndex)
            sub.keepFromRight (std::begin (*rightIndex), std::end (*rightIndex));
        if (sub.m_orientation != SIDE::RIGHT)
            sub.cleanUp (SIDE::RIGHT);
        std::unique_copy (std::begin (sub.m_right), std::end (sub.m_right), writeIterator);
    }

    
    template <typename WriteIterator>
    void getMatrix (WriteIterator writeLeft, WriteIterator writeRight) const
    {
        getMatrixFiltered (NULL, NULL, writeLeft, writeRight);
    }

    template <typename iterator_write_left, typename iterator_write_right>
    void getMatrixFiltered (const IKS::set<left_type>* indexOnLeft, const IKS::set<right_type>* indexOnRight, 
                            iterator_write_left writeLeft, iterator_write_right writeRight) const
    {
        if (!indexOnLeft && !indexOnRight)
        {
            std::copy (begin (m_left),  end (m_left),  writeLeft);
            std::copy (begin (m_right), end (m_right), writeRight);
            return;
        }

        // at least one of the indices is not NULL
        auto sub (*this);
        if (indexOnRight)
            sub.keepFromRight (begin (*indexOnRight), end (*indexOnRight));
        if (indexOnLeft)
            sub.keepFromLeft  (begin (*indexOnLeft),  end (*indexOnLeft));
        if (sub.m_orientation != SIDE::LEFT)
            sub.cleanUp (SIDE::LEFT);
        std::unique_copy (begin (sub.m_left),  end (sub.m_left),  writeLeft);
        std::unique_copy (begin (sub.m_right), end (sub.m_right), writeRight);
    }

private:

    mutable SIDE m_orientation;
    mutable bool m_isDirty;

    mutable std::vector<left_type> m_left;
    mutable std::vector<right_type> m_right;
};

