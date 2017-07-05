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
#include <random>



// #include "valgrind/callgrind.h"

#include "iks.hpp"
#include "relationMatrix_map.hpp"
#include "relationMatrix_vec.hpp"


using namespace std::chrono;





template <typename T>
T randomValue (T minValue, T maxValue)
{
    // static std::default_random_engine generator;
    // static std::uniform_int_distribution<T> distribution(minValue ,maxValue);
    // return distribution(generator);

    static std::random_device rd;     // only used once to initialise (seed) engine
    static std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(minValue,maxValue); // guaranteed unbiased

    return uni(rng);
}


template <typename Container, typename T>
void uniform (Container& container, T minValue, T maxValue)
{
    for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
    {
        (*it) = randomValue (minValue, maxValue);
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
    auto lastSrc = std::unique (begin (sources), end (sources));
    sources.erase (lastSrc, end (sources));
    std::sort (begin (sources), end (sources));

    targets.resize (size_t (targets.size () * toKeep));
    std::sort (begin (targets), end (targets));
    auto lastTgt = std::unique (begin (targets), end (targets));
    targets.erase (lastTgt, end (targets));
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



template <typename T>
void probeSortPermutation (std::string name, std::vector<T> vec)
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



    struct AA
    {
	AA () : vX (0) {}
	AA (int _v) : vX (_v) {}
	AA (const AA& other) : vX (other.vX) {}
	AA& operator= (const AA& other)
	{
	    vX = other.vX;
	    return *this;
	}
	bool operator== (const AA& other) { return vX == other.vX; }
	bool operator<  (const AA& other) { return vX < other.vX; }
	int vX;
    };

    struct BB
    {
	BB () : vY (0) {}
	BB (int _v) : vY (_v) {}
	BB (const BB& other) : vY (other.vY) {}
	BB& operator= (const BB& other)
	{
	    vY = other.vY;
	    return *this;
	}
	bool operator== (const BB& other) { return vY == other.vY; }
	bool operator<  (const BB& other) { return vY < other.vY; }
	int vY;
    };



    template <typename T>
    struct Comparison
    {
	bool operator () (const T& lhs, const T& rhs) const
	{
	    return false;
	}
    };
    
    template <>
	struct Comparison<AA>
    {
	bool operator () (const AA& lhs, const AA& rhs) const
	{
	    return lhs.vX < rhs.vX;
	}
    };
    
    template <>
	struct Comparison<BB>
    {
	bool operator () (const BB& lhs, const BB& rhs) const
	{
	    return lhs.vY < rhs.vY;
	}
    };
    


void test_different_types ()
{
    RelationMatrix2<AA, BB, Comparison<AA>, Comparison<BB> > relMat;
    relMat.addRelation (AA(4), BB(3));
    relMat.addRelation (AA(3), BB(3));
    relMat.addRelation (AA(7), BB(7));
    relMat.addRelation (AA(4), BB(4));
    relMat.addRelation (AA(2), BB(6));
    relMat.addRelation (AA(6), BB(2));

    
    IKS::set<AA> leftIdx;
    leftIdx.insert (AA(8));
    leftIdx.insert (AA(10));
    IKS::set<BB> rightIdx;
    rightIdx.insert (BB(3));
    rightIdx.insert (BB(6));
		     
    std::vector<AA> leftResult;
    relMat.getLeft (&leftIdx, &rightIdx, std::back_inserter (leftResult));
    
    std::vector<BB> rightResult;
    relMat.getRight (&leftIdx, &rightIdx, std::back_inserter (rightResult)); 
}


int main()
{
    test_different_types ();
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



    
//  std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000});
  std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000});
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

        std::vector<double> targets (count, 0);
        uniform (targets, maxRand0, maxRand1);

        // sources = {0,0,0,0,1,1,1,2,2,2,4,2,5,1,2};
        // targets = {5,4,4,3,3,3,7,7,6,9,9,9,9,9,9};
        
        // std::copy (sources.begin(), sources.end(), std::ostream_iterator<size_t>(std::cout, " "));
        // std::cout << std::endl;
        // std::copy (targets.begin(), targets.end(), std::ostream_iterator<size_t>(std::cout, " "));
        // std::cout << std::endl;

        std::cout << std::endl;
        RelationMatrix<size_t,double> relationMatrix;
        probeRelationMatrix (relationMatrix, "relationMatrix ", sources, targets, toKeep);
        RelationMatrix2<size_t, double> relationMatrix2;
        probeRelationMatrix (relationMatrix2, "relationMatrix2", sources, targets, toKeep);

        // RelationMatrix<double,size_t> relationMatrix;
        // probeRelationMatrix (relationMatrix, "relationMatrix ", targets, sources, toKeep);
        // RelationMatrix2<double,size_t> relationMatrix2;
        // probeRelationMatrix (relationMatrix2, "relationMatrix2", targets, sources, toKeep);

	// probeSortPermutation ("sort_permutation", sources);
    }
    return 0;

}

