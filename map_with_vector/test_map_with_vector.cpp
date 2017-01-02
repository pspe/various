#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <set>
#include <chrono>
#include <cassert>
#include <iterator>
#include <memory>
#include <map>

#include "valgrind/callgrind.h"


using namespace std::chrono;





template <typename T>
T randomValue (T minValue, T maxValue)
{
    static std::default_random_engine generator;
    static std::uniform_int_distribution<T> distribution(minValue ,maxValue);
    return distribution(generator);
}


template <typename Container, typename T>
void uniform (Container& container, T minValue, T maxValue)
{
    for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
    {
        (*it) = randomValue (minValue, maxValue);
    }
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




template <class InputIterator1, class InputIterator2, class OutputIterator>
void set_intersection_retain_duplicates_of_first (InputIterator1 first1, InputIterator1 last1,
                                                  InputIterator2 first2, InputIterator2 last2,
                                                  OutputIterator keepIndex)
{
    size_t idx (0);
    while (first1!=last1 && first2!=last2)
    {
        if (*first1<*first2)
        {
            ++first1;
            ++idx;
        }
        else if (*first2<*first1)
        {
            ++first2;
        }
        else
        {
            const auto& val = *first1;
            auto first1Upper = std::upper_bound (first1, last1, val);
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
template <typename CONTAINER>
void remove_key_value_duplicates (CONTAINER& keys, CONTAINER& values)
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
        auto itKeyUpper = std::upper_bound (itKey, lastKey, *itKey);
        auto distRange = itKeyUpper - itKey; // std::distance (itKey, itKeyUpper);
        // if (distRange > 1)
        {
            auto itValueLocalEnd = itValue + distRange;
            std::sort (itValue, itValueLocalEnd);
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


enum class ITERATOR_TYPE
{
    END, BEGIN
};

template <typename CONTAINER>
class vector_multimap_iterator : public std::iterator<std::random_access_iterator_tag, typename CONTAINER::value_type>
{
public:

    typedef typename CONTAINER::value_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type& reference;
    typedef const std::unique_ptr<value_type>& pointer;
    
    vector_multimap_iterator (const CONTAINER& container, ITERATOR_TYPE iterType = ITERATOR_TYPE::BEGIN)
        : m_position (0)
        , m_container (container)
    {
        if (iterType == ITERATOR_TYPE::END)
            m_position = container.size ();
    }

    vector_multimap_iterator (const vector_multimap_iterator<CONTAINER>& other)
        : m_position (other.m_position)
        , m_container (other.m_container)
    {}

    // vector_multimap_iterator& operator= (const vector_multimap_iterator<CONTAINER>& other)
    // {
    //     m_position = other.m_position;
    //     return *this;
    // }

    bool operator== (const vector_multimap_iterator<CONTAINER>& other) const
    {
        return m_position == other.m_position;
    }

    bool operator!= (const vector_multimap_iterator<CONTAINER>& other) const
    {
        return !((*this) == other);
    }
    
    vector_multimap_iterator& operator++ ()
    {
        ++m_position;
        return *this;
    }

    vector_multimap_iterator operator++ (int)
    {
        vector_multimap_iterator current = *this;
        ++(*this);
        return current;
    }

    vector_multimap_iterator& operator+= (int shift)
    {
        m_position += shift;
        return *this;
    }

    vector_multimap_iterator operator+ (int shift)
    {
        vector_multimap_iterator it = *this;
        it += shift;
        return it;
    }

    difference_type operator- (const vector_multimap_iterator& other)
    {
        return m_position - other.m_position;
    }

    
    value_type operator* () const
    {
        return std::make_pair (m_container.m_keys.at (m_position), m_container.m_values.at (m_position));
    }

    std::unique_ptr<value_type> operator-> () const
    {
        return std::make_unique<value_type> (std::make_pair (m_container.m_keys.at (m_position), m_container.m_values.at (m_position)));
    }
    
private:
    size_t m_position;
    const CONTAINER& m_container;
};


template <typename KEY, typename VALUE, typename COMPARE = std::less<KEY> >
class vector_multimap
{
public:
    typedef vector_multimap<KEY, VALUE, COMPARE> map_type;
    typedef std::pair<KEY,VALUE> value_type;
    typedef KEY key_type;
    typedef VALUE mapped_type;
    typedef vector_multimap_iterator<vector_multimap> iterator;
    typedef vector_multimap_iterator<vector_multimap> const_iterator;
    typedef typename iterator::difference_type difference_type;

    vector_multimap () : m_isDirty (false) {}
    
    vector_multimap& insert (const value_type& val)
    {
        m_keys.push_back (val.first);
        m_values.push_back (val.second);
        m_isDirty = true;
        return *this;
    }

    template <typename InputIterator>
    void insert (InputIterator first, InputIterator last)
    {
        size_t reserve = m_keys.size () + std::distance (first, last);
        m_keys.reserve (reserve);
        m_values.reserver (reserve);
        // std::for_each (first, last, [&](const typename InputIterator::value_type& val)
        //                {
        //                    m_keys.push_back (val.first);
        //                    m_values.push_back (val.second);
        //                });
        std::for_each (first, last, [&](const typename InputIterator::value_type& val)
                       {
                           m_keys.push_back (val.first);
                       });
        std::for_each (first, last, [&](const typename InputIterator::value_type& val)
                       {
                           m_values.push_back (val.second);
                       });
        m_isDirty = true;
        return *this;
    }

    iterator find (const key_type& k)
    {
        cleanUp ();
        auto it = std::lower_bound (m_keys.begin (), m_keys.end (), k);
        auto dist = std::distance (m_keys.begin (), it);
        return iterator (*this) + dist;
    }

    iterator lower_bound (const key_type& k)
    {
        return find (k);
    }

    iterator upper_bound (const key_type& k)
    {
        cleanUp ();
        auto it = std::upper_bound (m_keys.begin (), m_keys.end (), k);
        auto dist = std::distance (m_keys.begin (), it);
        return iterator (*this) + dist;
    }


    mapped_type& operator[] (const key_type& key)
    {
        cleanUp ();
        auto it = std::lower_bound (m_keys.begin (), m_keys.end (), key);
        auto dist = std::distance (m_keys.begin (), it);
        return m_values.at (dist);
    }

    mapped_type& operator[] (key_type&& key)
    {
        cleanUp ();
        auto it = std::lower_bound (m_keys.begin (), m_keys.end (), key);
        auto dist = std::distance (m_keys.begin (), it);
        return m_values.at (dist);
    }

    iterator erase (const_iterator itPos)
    {
        difference_type dist = std::distance (begin (), itPos);
        m_keys.erase (m_keys.begin () + dist);
        m_values.erase (m_values.begin () + dist);
        m_isDirty = true;
        return iterator (*this) + dist;
    }
    
    iterator erase (const iterator& first, const iterator& last)
    {
        difference_type dist = std::distance (begin (), first);
        difference_type span = std::distance (first, last);
        m_keys.erase (m_keys.begin () + dist, m_keys.begin () + dist + span);
        m_values.erase (m_values.begin () + dist, m_values.begin () + dist + span);
        m_isDirty = true;
        return iterator (*this) + dist + span;
    }
    
    
    size_t size () const
    {
        cleanUp ();
        return m_keys.size ();
    }

    bool empty () const { return m_keys.empty (); }

    void swap (map_type& other)
    {
        m_keys.swap (other.m_keys);
        m_values.swap (other.m_values);
        std::swap (m_isDirty, other.m_isDirty);
    }

    

    iterator begin () { cleanUp (); return vector_multimap_iterator<vector_multimap> (*this, ITERATOR_TYPE::BEGIN); }
    iterator end   () { cleanUp (); return vector_multimap_iterator<vector_multimap> (*this, ITERATOR_TYPE::END  ); }
    const_iterator begin () const { cleanUp (); return vector_multimap_iterator<vector_multimap> (*this, ITERATOR_TYPE::BEGIN); }
    const_iterator end   () const { cleanUp (); return vector_multimap_iterator<vector_multimap> (*this, ITERATOR_TYPE::END  ); }
    
private:

    void cleanUp () const
    {
        if (!m_isDirty)
            return;
        
        // apply the permutation
        std::vector<size_t> permutation = sort_permutation<key_type, COMPARE> (m_keys);
        m_keys = apply_permutation (m_keys, permutation);
        m_values = apply_permutation (m_values, permutation);
        // apply_permutation_in_place (m_keys, permutation);
        // apply_permutation_in_place (m_values, permutation);
        
        m_isDirty = false;
    }
    
    mutable std::vector<key_type> m_keys;
    mutable std::vector<mapped_type> m_values;

    mutable bool m_isDirty;


    friend class vector_multimap_iterator<vector_multimap>;
};



    template <typename T>
    struct UnConst
    {
        typedef T type;
        typedef const T const_type;
    };

    template <typename T>
    struct UnConst<const T>
    {
        typedef T type;
        typedef const T const_type;
    };



#ifndef CHOOSE_BOOLEAN
#define CHOOSE_BOOLEAN
    // see http://drdobbs.com/cpp/184401331?pgno=1 for how to avoid code duplication in iterators for const and non-const
    // "choose" template pattern
    template <bool flag, class IsTrue, class IsFalse> struct Choose;

    template <class IsTrue, class IsFalse>
    struct Choose<false, IsTrue, IsFalse> {
        typedef IsFalse type;
    };

    template <class IsTrue, class IsFalse>
    struct Choose<true, IsTrue, IsFalse> {
        typedef IsTrue type;
    };
#endif


namespace IKS
{

    // tests on gcc showed, that for large n, the std::set scales badly
    // the replacement IKS::set<> is based on the assumptions, 
    // that the sorting and uniquifying doesn't have to be done constantly
    // but only when the iterators are accessed and the existence of a
    // specific value is asked for
    // The tests have shown, that the std::vector based set scales nicely
    // (tests up to n=1e7 randomly inserted values have been done)
    // only for large n with a very large quantity of duplicates, 
    // the std::set performs slightly better.

    template <typename T>
    class set
    {
    public:
        typedef T value_type;
        typedef value_type& reference;
        typedef const value_type& const_reference;
        typedef value_type* pointer;
        typedef const value_type* const_pointer;
        typedef size_t difference_type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;

        set () 
            : m_data ()
            , m_isDirty (false)
        {}

        template <typename Iterator>
        set (Iterator it, Iterator itEnd) 
            : m_data ()
            , m_isDirty (true)
        {
            insert(it, itEnd);
        }

        // basics
        size_t          size    ()  const   { sortUnique (); return m_data.size (); }
        size_t          estimatedSize()  const   { return m_data.size (); }
        bool            empty   ()  const   { return m_data.empty (); }
        void            clear   ()          { m_data.clear (); m_isDirty = false; }

        bool operator== (const set& other) const
        {
            sortUnique ();
            other.sortUnique ();
            return m_data == other.m_data;
        }


        // iterators
        iterator        begin   ()          { sortUnique (); return m_data.begin  (); }
        iterator        end     ()          { sortUnique (); return m_data.end  (); }
        const_iterator  begin   ()  const   { sortUnique (); return m_data.begin  (); }
        const_iterator  end     ()  const   { sortUnique (); return m_data.end  (); }

        // insert
        iterator        insert  (const value_type& val)                 { m_isDirty = true; return m_data.insert (m_data.end (), val); }
        template <typename Iterator>    
        void            insert  (Iterator from, Iterator to)            { m_isDirty = true; m_data.insert (m_data.end (), from, to); }
        //template <typename Iterator>
        //iterator        insert  (Iterator iter, const value_type& val)  { m_isDirty = true; return m_data.insert (iter, val); }
        template <typename Iterator>
        iterator        insert  (Iterator iter, const value_type& val)  { m_isDirty = true; m_data.push_back (val); return m_data.begin (); }

        // find
        iterator        find    (const value_type& val)         { sortUnique (); return std::lower_bound (m_data.begin (), m_data.end (), val); }
        const_iterator  find    (const value_type& val) const   { sortUnique (); return std::lower_bound (m_data.begin (), m_data.end (), val); }

        bool            exists  (const value_type& val) const   { sortUnique (); return std::binary_search (m_data.begin (), m_data.end (), val); }


    protected:

        void            sortUnique ()   const    
        { 
            if (!m_isDirty)
                return;
            std::sort (m_data.begin (), m_data.end ());
            m_data.erase (std::unique (m_data.begin (), m_data.end ()), m_data.end ());
            m_isDirty = false;
        }

    private:
        mutable std::vector<T>  m_data;
        mutable bool            m_isDirty;
    };

} // namespace IKS













template <typename L, typename R = L>
class RelationMatrix 
{
private:
    struct DontCheckSecondIndex
    {
        template <typename Container, typename T>
        static inline bool exists (Container /*container*/, const T& /*position*/) 
        {
            return true;                    
        }

    };


    struct CheckSecondIndex
    {
        template <typename Container, typename T>
        static inline bool exists (Container container, const T& position) 
        {
            assert(container != NULL);
            return container->exists (position);
//					return container->find (position)!=container->end();                    
        }

    };



public:

    typedef typename std::vector<R> R_secondary_direction_type;
    typedef typename std::map<L, R_secondary_direction_type> L_primary_direction_type;
    typedef typename std::vector<L> L_secondary_direction_type;
    typedef typename std::map<R, L_secondary_direction_type> R_primary_direction_type;

    RelationMatrix (bool buildLeftSideOnAddRelation = true, bool buildRightSideOnAddRelation = false) 
        : m_rightSideComplete(buildRightSideOnAddRelation)
        , m_leftSideComplete(buildLeftSideOnAddRelation)
        , m_buildLeftSideOnAddRelation(buildLeftSideOnAddRelation)
        , m_buildRightSideOnAddRelation(buildRightSideOnAddRelation)
    {
        assert (m_buildLeftSideOnAddRelation || m_buildRightSideOnAddRelation);
        m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        m_isDirtyPtrs[1] = &m_rightPrimaryElements;
    }


    RelationMatrix (const RelationMatrix& other) 
        : m_buildLeftSideOnAddRelation (other.m_buildLeftSideOnAddRelation)
        , m_buildRightSideOnAddRelation (other.m_buildRightSideOnAddRelation)
        , m_rightSideComplete (other.m_rightSideComplete)
        , m_leftSideComplete (other.m_leftSideComplete)
        , m_leftPrimaryElements (other.m_leftPrimaryElements)
        , m_rightPrimaryElements (other.m_rightPrimaryElements)
    {
        if (&other.m_leftPrimaryElements == other.m_isDirtyPtrs[0] || 
            &other.m_leftPrimaryElements == other.m_isDirtyPtrs[1])
        {
            m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        }
        if (&other.m_rightPrimaryElements == other.m_isDirtyPtrs[0] || 
            &other.m_rightPrimaryElements == other.m_isDirtyPtrs[1])
        {
            m_isDirtyPtrs[1] = &m_rightPrimaryElements;
        }
    }


    RelationMatrix& operator= (const RelationMatrix& other) 
    {
        m_buildLeftSideOnAddRelation = other.m_buildLeftSideOnAddRelation;
        m_buildRightSideOnAddRelation = other.m_buildRightSideOnAddRelation;
        m_rightSideComplete = other.m_rightSideComplete;
        m_leftSideComplete = other.m_leftSideComplete;
        m_leftPrimaryElements = other.m_leftPrimaryElements;
        m_rightPrimaryElements = other.m_rightPrimaryElements;

        if (&other.m_leftPrimaryElements == other.m_isDirtyPtrs[0] || 
            &other.m_leftPrimaryElements == other.m_isDirtyPtrs[1])
        {
            m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        }
        if (&other.m_rightPrimaryElements == other.m_isDirtyPtrs[0] || 
            &other.m_rightPrimaryElements == other.m_isDirtyPtrs[1])
        {
            m_isDirtyPtrs[1] = &m_rightPrimaryElements;
        }
        return *this;
    }



    void setBuildLeftSideOnAddRelation  (bool left = true) { m_buildLeftSideOnAddRelation = left; }
    void setBuildRightSideOnAddRelation (bool right= true) { m_buildRightSideOnAddRelation = right; }
    void clear ()
    {
        m_leftPrimaryElements.clear();
        m_rightPrimaryElements.clear();
        m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        m_isDirtyPtrs[1] = &m_rightPrimaryElements;
        m_leftSideComplete  = m_buildLeftSideOnAddRelation;
        m_rightSideComplete = m_buildRightSideOnAddRelation;
    }

    size_t size () const 
    {
        cleanUp (m_leftPrimaryElements);
        size_t count(0);
        for (typename L_primary_direction_type::const_iterator it = m_leftPrimaryElements.begin(), itEnd = m_leftPrimaryElements.end();
             it != itEnd; ++it)
        {
            const R_secondary_direction_type& line = it->second;
            count += line.size ();
        }
        return count;
    }

    size_t sizeLeft () const
    {
        cleanUp (m_leftPrimaryElements);
        return m_leftPrimaryElements.size ();
    }

    size_t sizeRight () const
    {
        cleanUp (m_rightPrimaryElements);
        return m_rightPrimaryElements.size ();
    }

    void addRelation (L left, R right)
    {
        m_leftSideComplete = false;
        m_rightSideComplete = false;
        if (m_buildLeftSideOnAddRelation)
        {
            m_leftPrimaryElements[left].push_back(right);
            m_leftSideComplete = true;
        }
        if (m_buildRightSideOnAddRelation)
        {
            m_rightPrimaryElements[right].push_back(left);
            m_rightSideComplete = true;
        }

        m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        m_isDirtyPtrs[1] = &m_rightPrimaryElements;
    }


    template <typename SOURCE, typename DEST>
    void crossBuild (const SOURCE& source, DEST& dest) const
    {
        dest.clear ();
        for (typename SOURCE::const_iterator itSrcLine = source.begin(), itSrcLineEnd = source.end(); 
             itSrcLine != itSrcLineEnd; ++itSrcLine)
        {
            typename SOURCE::key_type key = itSrcLine->first;
            for (typename SOURCE::mapped_type::const_iterator it = itSrcLine->second.begin(),
                     itEnd = itSrcLine->second.end(); it != itEnd; ++it)
            {
                dest[*it].push_back (key);
            }
        }
        m_rightSideComplete = true;
    }


    template <typename T>
    void cleanUp (T& data) const
    {   
        assert (m_leftSideComplete || m_rightSideComplete);

        if (&data != m_isDirtyPtrs[0] && &data != m_isDirtyPtrs[1]) // neither left nor right side are dirty
            return; // get me out from here

        if (!m_leftSideComplete && &data == &m_leftPrimaryElements)
        {
            cleanUp (m_rightPrimaryElements);
            crossBuild (m_rightPrimaryElements, m_leftPrimaryElements);
        }
        else if (!m_rightSideComplete && &data == &m_rightPrimaryElements)
        {
            cleanUp (m_leftPrimaryElements);
            crossBuild (m_leftPrimaryElements, m_rightPrimaryElements);
        }
        for (typename T::iterator it = const_cast<typename UnConst<T>::type&>(data).begin(), itEnd = const_cast<typename UnConst<T>::type&>(data).end(); it != itEnd; ++it)
        {
            typename T::mapped_type& line = it->second;
            std::sort(line.begin(), line.end());
            line.erase (std::unique(line.begin(), line.end()), line.end());
        }

        if (&data == m_isDirtyPtrs[0])
            m_isDirtyPtrs[0] = 0;
        else if (&data == m_isDirtyPtrs[1])
            m_isDirtyPtrs[1] = 0;
    }


    template <typename Iterator>
    void keepFromLeft (Iterator begin, Iterator end)
    {
        cleanUp (m_leftPrimaryElements);
        m_rightPrimaryElements.clear ();
        m_isDirtyPtrs[1] = &m_rightPrimaryElements;
        keep (m_leftPrimaryElements, begin, end);
        m_leftSideComplete = true;
        m_rightSideComplete = false;
//				m_buildRightSideOnAddRelation = false;
    }

    template <typename Iterator>
    void keepFromRight (Iterator begin, Iterator end)
    {
        cleanUp (m_rightPrimaryElements);
        m_leftPrimaryElements.clear ();
        m_isDirtyPtrs[0] = &m_leftPrimaryElements;
        keep (m_rightPrimaryElements, begin, end);
        m_rightSideComplete = true;
        m_leftSideComplete = false;
//				m_buildLeftSideOnAddRelation = false;
    }


    template <typename WriteIterator>
    void getLeft (const IKS::set<L>* indexLeft, const IKS::set<R>* indexRight, WriteIterator writeIterator) const 
    {
        getVector (m_leftPrimaryElements, m_rightPrimaryElements, indexLeft, indexRight, writeIterator);
    }

    template <typename WriteIterator>
    void getRight (const IKS::set<L>* indexLeft, const IKS::set<R>* indexRight, WriteIterator writeIterator) const 
    {
        getVector(m_rightPrimaryElements, m_leftPrimaryElements, indexRight, indexLeft, writeIterator);
    }



    template <typename WriteIterator>
    void getMatrix (WriteIterator writeLeft, WriteIterator writeRight) const
    {
        //                Set<L>* keysLeft = new Set<L>();
        //                keys (m_leftPrimaryElements, std::inserter(*keysLeft, keysLeft->begin()));
        if (m_leftSideComplete)
            getMatrix<DontCheckSecondIndex> (m_leftPrimaryElements, (IKS::set<L>*)NULL,  (IKS::set<L>*)NULL, writeLeft, writeRight);
        else if (m_rightSideComplete)
            getMatrix<DontCheckSecondIndex> (m_rightPrimaryElements, (IKS::set<L>*)NULL,  (IKS::set<L>*)NULL, writeRight, writeLeft);
        //                delete keysLeft;
    }

    template <typename WriteIterator>
    void getMatrixFiltered (const IKS::set<L>* indexOnLeft, const IKS::set<R>* indexOnRight, 
                            WriteIterator writeLeft, WriteIterator writeRight) const
    {
        if (indexOnLeft == NULL && indexOnRight == NULL)
            return getMatrix (writeLeft, writeRight);

        if (indexOnLeft == NULL)
            return getMatrix<DontCheckSecondIndex> (m_rightPrimaryElements, indexOnRight, indexOnLeft, writeRight, writeLeft);

        if (indexOnRight == NULL)
            return getMatrix<DontCheckSecondIndex> (m_leftPrimaryElements, indexOnLeft, indexOnRight, writeLeft, writeRight);

        if (m_leftSideComplete)
            getMatrix<CheckSecondIndex> (m_leftPrimaryElements, indexOnLeft, indexOnRight, writeLeft, writeRight);
        else if (m_rightSideComplete)
            getMatrix<CheckSecondIndex> (m_rightPrimaryElements, indexOnRight, indexOnLeft, writeRight, writeLeft);
    }



private:

    template <typename Elements, typename Iterator>
    void keep (Elements& elements, Iterator begin, Iterator end)
    {
        Iterator it = begin;
        typename Elements::iterator itElem    (elements.begin());
        typename Elements::iterator itElemEnd (elements.end());
        for (; itElem != itElemEnd && it != end; )
        {
            if (itElem->first < *it)
            {
                elements.erase (itElem++);
            }
            else if (*it < itElem->first)
            {
                ++it;
            }
            else
            {
                ++itElem;
                ++it;
            }
        }
        elements.erase (itElem, itElemEnd);
    }



    template <typename ItIndex0, typename ItIndex1, typename Elements, typename WriteIterator>
    void project (ItIndex0 it0, ItIndex0 it0End, ItIndex1 it1, ItIndex1 it1End, Elements& elements, WriteIterator itWrite) const
    {
        typedef typename std::iterator_traits<ItIndex0>::value_type value_type;
//				std::set<value_type, std::less<value_type>, MemoryPoolAllocator<value_type> > tmpSet;
        std::set<value_type> tmpSet;

        if (it1 == it1End)
            return;

        for (; it1 != it1End; ++it1)
        {
            typename Elements::const_iterator itLine = elements.find ((*it1));
            if (itLine == elements.end())
                continue;

            std::set_intersection (
                itLine->second.begin(), 
                itLine->second.end(), 
                it0, 
                it0End, 
                std::inserter(tmpSet, tmpSet.begin()));
        }
        std::copy (tmpSet.begin (), tmpSet.end(), itWrite);
    }


    template <typename ItIndex, typename Elements, typename WriteIterator>
    void project (ItIndex it, ItIndex itEnd, Elements& elements, WriteIterator itWrite) const
    {
        typedef typename Elements::mapped_type::value_type value_type;
        std::set<value_type> tmpSet;

        if (it == itEnd)
            return;

        for (; it != itEnd; ++it)
        {
            typename Elements::const_iterator itLine = elements.find ((*it));
            if (itLine == elements.end())
                continue;

            tmpSet.insert (itLine->second.begin(), itLine->second.end());
        }
        std::copy (tmpSet.begin (), tmpSet.end(), itWrite);
    }



    template <
        typename PrimaryDirectionType0, 
        typename PrimaryDirectionType1, 
        typename T0,
        typename T1,
        typename WriteIterator>
    void getVector (const PrimaryDirectionType0& elements0, const PrimaryDirectionType1& elements1, 
                    const IKS::set<T0>* index0, 
                    const IKS::set<T1>* index1, 
                    WriteIterator writeIterator) const
    {
        if (index0 == NULL)
        {
            if (index1 == NULL)
            {
                cleanUp (elements0);
                std::copy (const_key_iterator(elements0, elements0.begin()), const_key_iterator(elements0, elements0.end()), writeIterator);
                return;
            }

            cleanUp (elements1);
            project (
                index1->begin(), index1->end(), 
                elements1, writeIterator
                );
            return; 
        }
        if (index1 == NULL)
        {
            cleanUp (elements0);
            std::set_intersection (	const_key_iterator(elements0, elements0.begin()), 
                                        const_key_iterator(elements0, elements0.end()), 
                                        index0->begin(),
                                        index0->end(),
                                        writeIterator);
            return;
        }

        cleanUp (elements1);
        project (index0->begin(), index0->end(), index1->begin(), index1->end(), elements1, writeIterator);
    }




    template <typename Check2ndIndexPolicy, typename PrimaryDirectionType, typename WriteIterator>
    void getMatrix (const PrimaryDirectionType& elements, 
                    const IKS::set<typename PrimaryDirectionType::key_type>* index0, 
                    const IKS::set<typename PrimaryDirectionType::mapped_type::value_type>* index1, 
                    WriteIterator writeLeft, 
                    WriteIterator writeRight) const 
    {
        typedef typename PrimaryDirectionType::key_type first_type;
        typedef typename PrimaryDirectionType::mapped_type::value_type second_type;

        cleanUp (elements);
        if (index0 == NULL)
        {
            for (typename PrimaryDirectionType::const_iterator itLine = elements.begin(), 
                     itLineEnd = elements.end(); itLine != itLineEnd; ++itLine)
            {
                const first_type& position = itLine->first;
                const typename PrimaryDirectionType::mapped_type& line = itLine->second;
                for (typename PrimaryDirectionType::mapped_type::const_iterator itElm = line.begin(), itElmEnd = line.end(); itElm != itElmEnd; ++itElm)
                {
                    const second_type& posElm = (*itElm);
                    if (Check2ndIndexPolicy::exists(index1, posElm))
                    {
                        (*writeLeft)  = position;
                        (*writeRight) = posElm;
                        ++writeLeft;
                        ++writeRight;
                    }
                }
            }
            return;
        }
        for (typename IKS::set<first_type>::const_iterator itIdx = index0->begin(), itIdxEnd = index0->end(); itIdx != itIdxEnd; ++itIdx)
        {
            const first_type& position = (*itIdx);
            typename PrimaryDirectionType::const_iterator itLine = elements.find (position);
            if (itLine == elements.end())
                continue;
            const typename PrimaryDirectionType::mapped_type& line = itLine->second;
            for (typename PrimaryDirectionType::mapped_type::const_iterator itElm = line.begin(), itElmEnd = line.end(); itElm != itElmEnd; ++itElm)
            {
                const second_type& posElm = (*itElm);
                if (Check2ndIndexPolicy::exists(index1, posElm))
                {
                    (*writeLeft)  = position;
                    (*writeRight) = posElm;
                    ++writeLeft;
                    ++writeRight;
                }
            }
        }
    }


    template <typename PrimaryDirectionType, typename WriteIterator>
    void keys (const PrimaryDirectionType& elements, WriteIterator writeIterator) const 
    {
        for (typename PrimaryDirectionType::const_iterator it = elements.begin(), itEnd = elements.end(); it != itEnd; ++it)
        {
            (*writeIterator) = it->first;
            ++writeIterator;
        }
    }

private:
    mutable L_primary_direction_type m_leftPrimaryElements;
    mutable R_primary_direction_type m_rightPrimaryElements;

    mutable void* m_isDirtyPtrs[2];

    mutable bool  m_rightSideComplete;
    mutable bool  m_leftSideComplete;

    bool m_buildLeftSideOnAddRelation;
    bool m_buildRightSideOnAddRelation;
};







template <typename T>
class RelationMatrix2 
{
private:

    enum class SIDE
    {
        LEFT, RIGHT
    };

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

    size_t sizeLeft (SIDE eSide) const
    {
        cleanUp (eSide);
        std::vector<T> tmp (eSide == SIDE::LEFT ? m_left : m_right);
        std::sort (begin (tmp), end (tmp));
        return std::unique (begin (tmp), end (tmp)) - begin (tmp);
    }

    
    size_t sizeLeft () const
    {
        return size (SIDE::LEFT);
    }

    size_t sizeRight () const
    {
        return size (SIDE::RIGHT);
    }

    void addRelation (T left, T right)
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

    template <typename CONTAINER>
    void cleanUp (CONTAINER& keys, CONTAINER& values) const
    {
        // CALLGRIND_START_INSTRUMENTATION;
        // CALLGRIND_TOGGLE_COLLECT;

        typedef typename CONTAINER::value_type value_type;
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
        std::vector<size_t> permutation = sort_permutation<value_type, std::less<value_type> >  (keys);

        // apply the sort permutation on keys and values
        keys   = apply_permutation (keys,   permutation);
        values = apply_permutation (values, permutation);

        // apply_permutation (keys, values, permutation);
        
        // apply_permutation_in_place (keys,   permutation);
        // apply_permutation_in_place (values, permutation);

        // check for key-value duplicates
        // remove_key_value_duplicates_2 (keys, values);
        remove_key_value_duplicates (keys, values);

        // CALLGRIND_TOGGLE_COLLECT;
        // CALLGRIND_STOP_INSTRUMENTATION;

    }

    
    void cleanUp (SIDE eSide) const
    {
        if (eSide == SIDE::LEFT)
        {
            if (m_orientation == SIDE::LEFT && !m_isDirty)
                return;
            cleanUp (m_left, m_right);
            m_isDirty = false;
            m_orientation = SIDE::LEFT;
            return;
        }
        // eSide == SIDE::RIGHT
        if (m_orientation == SIDE::RIGHT && !m_isDirty)
            return;
        cleanUp (m_right, m_left);
        m_isDirty = false;
        m_orientation = SIDE::RIGHT;
    }
    


    
    template <typename Iterator>
    void keepFromSide (Iterator first, Iterator last, SIDE eSide)
    {
        assert (std::is_sorted (first, last));
        std::vector<size_t> keepIndex;
        cleanUp (eSide);
        auto& left  = eSide == SIDE::LEFT ? m_left : m_right;
        auto& right = eSide == SIDE::LEFT ? m_right : m_left;
        keepIndex.reserve (left.size ());
        set_intersection_retain_duplicates_of_first (begin (left), end (left), first, last, std::back_inserter (keepIndex));
        left = apply_permutation (left, keepIndex);
        right = apply_permutation (right, keepIndex);
        m_orientation = eSide;
    }

    
    template <typename Iterator>
    void keepFromLeft (Iterator first, Iterator last)
    {
        keepFromSide (first, last, SIDE::LEFT);
    }

    template <typename Iterator>
    void keepFromRight (Iterator first, Iterator last)
    {
        keepFromSide (first, last, SIDE::RIGHT);
    }

    template <typename WriteIterator>
    void getLeft (const IKS::set<T>* indexLeft, const IKS::set<T>* indexRight, WriteIterator writeIterator) const
    {
        getVector (SIDE::LEFT, indexLeft, indexRight, writeIterator);
    }

    template <typename WriteIterator>
    void getRight (const IKS::set<T>* indexLeft, const IKS::set<T>* indexRight, WriteIterator writeIterator) const
    {
        getVector (SIDE::RIGHT, indexLeft, indexRight, writeIterator);
    }

    template <typename WriteIterator>
    void getMatrix (WriteIterator writeLeft, WriteIterator writeRight) const
    {
        getMatrixFiltered (NULL, NULL, writeLeft, writeRight);
    }

    template <typename WriteIterator>
    void getMatrixFiltered (const IKS::set<T>* indexOnLeft, const IKS::set<T>* indexOnRight, 
                            WriteIterator writeLeft, WriteIterator writeRight) const
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



    template <typename WriteIterator>
    void getVector (SIDE eSide, 
                    const IKS::set<T>* leftIndex, 
                    const IKS::set<T>* rightIndex, 
                    WriteIterator writeIterator) const
    {
        if (!leftIndex && !rightIndex)
        {
            cleanUp (eSide);
            if (eSide == SIDE::LEFT)
                std::unique_copy (begin (m_left), end (m_left), writeIterator);
            else
                std::unique_copy (begin (m_right), end (m_right), writeIterator);
            return;
        }

        auto sub (*this);
        if (eSide == SIDE::LEFT)
        {
            if (rightIndex)
                sub.keepFromRight (begin (*rightIndex), end (*rightIndex));
            if (leftIndex)
                sub.keepFromLeft  (begin (*leftIndex),  end (*leftIndex));
            if (sub.m_orientation != SIDE::LEFT)
                sub.cleanUp (SIDE::LEFT);
            std::unique_copy (begin (sub.m_left), end (sub.m_left), writeIterator);
            return;
        }
        // eSide == SIDE::RIGHT
        if (leftIndex)
            sub.keepFromLeft  (begin (*leftIndex),  end (*leftIndex));
        if (rightIndex)
            sub.keepFromRight (begin (*rightIndex), end (*rightIndex));
        if (sub.m_orientation != SIDE::RIGHT)
            sub.cleanUp (SIDE::RIGHT);
        std::unique_copy (begin (sub.m_right), end (sub.m_right), writeIterator);
    }



private:

    mutable SIDE m_orientation;
    mutable bool m_isDirty;

    mutable std::vector<T> m_left;
    mutable std::vector<T> m_right;
};









template <typename map_type>
void probe (map_type& testMap, std::string name, auto keys, auto values, size_t toFind)
{
    // map_type
    const int width (15);
    const int precision (6);
    std::cout << std::setw (width) << keys.size () << " : " << std::setw (width) << name << " : " << std::flush;
    high_resolution_clock::time_point startTime = high_resolution_clock::now ();
    auto itValues = begin (values);
    for_each (begin (keys), end (keys), [&testMap, &itValues](size_t key)
              {
                  testMap.insert (std::make_pair (key, (*itValues++)));
              });

    high_resolution_clock::time_point findTime = high_resolution_clock::now ();
    size_t currFind = 0;
    for (auto k : keys)
    {
        if (currFind >= toFind)
            break;

        bool exists = testMap.find (k) != testMap.end ();
        if (exists)
            ++currFind;
    }
    high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
    duration<double> time_span_insert = duration_cast<duration<double>> (findTime-startTime);
    duration<double> time_span_find = duration_cast<duration<double>> (endTime-findTime);
    std::cout << "time insert = " << std::fixed << std::setprecision (precision) << time_span_insert.count () << "  time find = " << std::setprecision (precision) << time_span_find.count () << std::setw (width) << "   found = " << std::setw (width) << currFind << std::endl;
}



template <typename rel_mat>
void probeRelationMatrix (rel_mat& testMatrix, std::string name, auto sources, auto targets, double toKeep)
{
    // map_type
    const int width (12);
    const int precision (6);
    std::cout << std::setw (width) << sources.size () << " : " << std::setw (width) << name << " : " << std::flush;

    high_resolution_clock::time_point startTime = high_resolution_clock::now ();
    auto itTarget = begin (targets);
    for_each (begin (sources), end (sources), [&testMatrix, &itTarget](size_t key)
              {
                  testMatrix.addRelation (key, (*itTarget++));
              });

    high_resolution_clock::time_point insertTime = high_resolution_clock::now ();
    //testMatrix.cleanUp (RelationMatrix2<size_t>::SIDE::LEFT);
    size_t initialSize = testMatrix.size ();
    high_resolution_clock::time_point insertTime_end = high_resolution_clock::now ();
    // size_t currFind = 0;

    // decltype (sources) resultSources;
    // decltype (targets) resultTargets;
    // testMatrix.getMatrix (std::back_inserter (resultSources), std::back_inserter (resultTargets));

    // std::cout << std::endl;
    // std::cout << "initial" << std::endl;
    // std::copy (resultSources.begin(), resultSources.end(), std::ostream_iterator<size_t>(std::cout, " "));
    // std::cout << std::endl;
    // std::copy (resultTargets.begin(), resultTargets.end(), std::ostream_iterator<size_t>(std::cout, " "));
    // std::cout << std::endl;

    sources.resize (size_t (sources.size () * toKeep));
    std::sort (begin (sources), end (sources));
    auto last = std::unique (begin (sources), end (sources));
    sources.erase (last, end (sources));
    std::sort (begin (sources), end (sources));

    targets.resize (size_t (targets.size () * toKeep));
    std::sort (begin (targets), end (targets));
    last = std::unique (begin (targets), end (targets));
    targets.erase (last, end (targets));
    std::sort (begin (targets), end (targets));
    
    high_resolution_clock::time_point findTime = high_resolution_clock::now ();

    // std::cout << std::endl;
    // std::cout << "reduced sources" << std::endl;
    // std::copy (sources.begin(), sources.end(), std::ostream_iterator<size_t>(std::cout, " "));
    // std::cout << std::endl;
    
    testMatrix.keepFromLeft (begin (sources), end (sources));
    testMatrix.keepFromRight (begin (targets), end (targets));

    // resultSources.clear ();
    // resultTargets.clear ();
    // testMatrix.getMatrix (std::back_inserter (resultSources), std::back_inserter (resultTargets));

    // std::cout << std::endl;
    // std::cout << "kept" << std::endl;
    // std::copy (resultSources.begin(), resultSources.end(), std::ostream_iterator<size_t>(std::cout, " "));
    // std::cout << std::endl;
    // std::copy (resultTargets.begin(), resultTargets.end(), std::ostream_iterator<size_t>(std::cout, " "));
    // std::cout << std::endl;
    // std::cout << std::endl;

    
    high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
    duration<double> time_span_insert = duration_cast<duration<double>> (insertTime-startTime);
    duration<double> time_span_clean = duration_cast<duration<double>> (insertTime_end-insertTime);
    duration<double> time_span_find = duration_cast<duration<double>> (endTime-findTime);
    std::cout << "t_insert = " << std::fixed << std::setprecision (precision) << time_span_insert.count ()
              << " t_clean = " << std::fixed << std::setprecision (precision) << time_span_clean.count ()
              << " t_find = " << std::fixed << std::setprecision (precision) << time_span_find.count ()  << std::setw (width)
              << " initial = " << std::setw (width) << initialSize << std::setw (width)
              << " kept = " << std::setw (width) << testMatrix.size () << " src size = " << sources.size () << std::endl;
}




void probeSortPermutation (std::string name, std::vector<size_t> vec)
{
    // map_type
    const int width (12);
    const int precision (6);
    std::cout << std::setw (width) << vec.size () << " : " << std::setw (width) << name << " : " << std::flush;

    high_resolution_clock::time_point startTime = high_resolution_clock::now ();

    auto sortedVec = sort_permutation<size_t, std::less<size_t>> (vec);
    
    high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
    duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
    std::cout << " t = " << std::setprecision (precision) << time_span.count () << std::endl;
}



int main()
{
    // typedef vector_multimap<size_t,std::string> VecMapType;
    // VecMapType vecMap;
    // vecMap.insert (std::make_pair (3, std::string ("hallo")));
    // vecMap.insert (std::make_pair (1, std::string ("test")));
    // vecMap.insert (std::make_pair (4, std::string ("wir")));
    // vecMap.insert (std::make_pair (3, std::string ("auch")));

    // for (auto it = begin (vecMap), itEnd = end (vecMap); it != itEnd; ++it)
    // {
    //     const auto& entry (*it);
    //     std::cout << "key = " << entry.first << "  val = " << entry.second << std::endl;
    // }

    // VecMapType::iterator itFind = vecMap.find (3);
    // std::cout << "found = " << itFind->first << " with value " << itFind->second << std::endl;

    // vecMap.erase (vecMap.lower_bound (3), vecMap.upper_bound (3));
    
    // for (auto it = begin (vecMap), itEnd = end (vecMap); it != itEnd; ++it)
    // {
    //     const auto& entry (*it);
    //     std::cout << "key = " << entry.first << "  val = " << entry.second << std::endl;
    // }

    
    
    // return 0;
    
    
    
//     std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000});

//     typedef size_t key_type;
// //    typedef std::vector<size_t> value_type;
//     typedef size_t value_type;
    
//     size_t toFind = 1000000000;
//     for (auto count : counts)
//     {
//         std::vector<key_type> keys (count);
//         impl_iota (begin (keys), end (keys), 0);
// //        std::vector<value_type> values (count, value_type ({2,3}));
//         std::vector<value_type> values (count);
//         impl_iota (begin (values), end (values), 0);
        
//         vector_multimap<key_type, value_type> testVecMap;
//         probe (testVecMap, "vector_multimap", keys, values, toFind);

//         std::map<key_type, value_type> testMap;
//         probe (testMap, "std::map", keys, values, toFind);

//     }
//     return 0;


  std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000});
//    std::vector<size_t> counts ({5,10,50,100, 1000, 10000, 100000, 300000});
//    std::vector<size_t> counts ({10});
//    std::vector<size_t> counts ({ 10000000});
//    std::vector<size_t> counts ({ 5,10,20 });

    double randFactor = 100.0;
    double toKeep = 0.5;
    for (auto count : counts)
    {
        size_t maxRand0 = size_t (count * randFactor);
        size_t maxRand1 = size_t (count * randFactor);
        std::vector<size_t> sources (count, 0);
        uniform (sources, size_t(0), maxRand0);

        std::vector<size_t> targets (count, 0);
        uniform (targets, maxRand0, maxRand1);

        // sources = {0,0,0,0,1,1,1,2,2,2,4,2,5,1,2};
        // targets = {5,4,4,3,3,3,7,7,6,9,9,9,9,9,9};
        
        // std::copy (sources.begin(), sources.end(), std::ostream_iterator<size_t>(std::cout, " "));
        // std::cout << std::endl;
        // std::copy (targets.begin(), targets.end(), std::ostream_iterator<size_t>(std::cout, " "));
        // std::cout << std::endl;

        std::cout << std::endl;
        RelationMatrix<size_t,size_t> relationMatrix;
        probeRelationMatrix (relationMatrix, "relationMatrix ", sources, targets, toKeep);
        RelationMatrix2<size_t> relationMatrix2;
        probeRelationMatrix (relationMatrix2, "relationMatrix2", sources, targets, toKeep);
        // probeSortPermutation ("sort_permutation", sources);
    }
    return 0;

}

