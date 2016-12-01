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


template <typename CONTAINER, typename COMPARE>
struct CompareFunctor
{
    CompareFunctor (const CONTAINER& _container)
        : compare ()
        , container (_container)
          
    {
    }
    
    template <typename T>
    bool operator() (const T& lhs, const T& rhs)
    {
        return compare (container.at (lhs), container.at (rhs));
    }

    COMPARE compare;
    const CONTAINER& container;
};


// find the sort permutation from the vector of keys
template <typename T, typename COMPARE>
std::vector<std::size_t> sort_permutation (const std::vector<T>& vec)
{
    std::vector<size_t> permutation (vec.size ());
    impl_iota (permutation.begin (), permutation.end (), 0);
//    std::sort (permutation.begin (), permutation.end (), COMPARE ());
    std::sort (permutation.begin (), permutation.end (), CompareFunctor<const std::vector<T>, COMPARE> (vec));
    return permutation;
}


template <typename T>
std::vector<T> apply_permutation(const std::vector<T>& vec, const std::vector<std::size_t>& p)
{
    std::vector<T> sorted_vec (vec.size());
    std::transform (p.begin(), p.end(), sorted_vec.begin(),
                    [&](std::size_t i){ return vec[i]; });
    return sorted_vec;
}


template <typename T>
void apply_permutation_in_place (std::vector<T>& vec, const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
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
class vector_map_iterator : public std::iterator<std::random_access_iterator_tag, typename CONTAINER::value_type>
{
public:

    typedef typename CONTAINER::value_type value_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type& reference;
    typedef const std::unique_ptr<value_type>& pointer;
    
    vector_map_iterator (const CONTAINER& container, ITERATOR_TYPE iterType = ITERATOR_TYPE::BEGIN)
        : m_position (0)
        , m_container (container)
    {
        if (iterType == ITERATOR_TYPE::END)
            m_position = container.size ();
    }

    vector_map_iterator (const vector_map_iterator<CONTAINER>& other)
        : m_position (other.m_position)
        , m_container (other.m_container)
    {}

    // vector_map_iterator& operator= (const vector_map_iterator<CONTAINER>& other)
    // {
    //     m_position = other.m_position;
    //     return *this;
    // }

    bool operator== (const vector_map_iterator<CONTAINER>& other) const
    {
        return m_position == other.m_position;
    }

    bool operator!= (const vector_map_iterator<CONTAINER>& other) const
    {
        return !((*this) == other);
    }
    
    vector_map_iterator& operator++ ()
    {
        ++m_position;
        return *this;
    }

    vector_map_iterator operator++ (int)
    {
        vector_map_iterator current = *this;
        ++(*this);
        return current;
    }

    vector_map_iterator& operator+= (int shift)
    {
        m_position += shift;
        return *this;
    }

    vector_map_iterator operator+ (int shift)
    {
        vector_map_iterator it = *this;
        it += shift;
        return it;
    }

    difference_type operator- (const vector_map_iterator& other)
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
class vector_map
{
public:
    typedef vector_map<KEY, VALUE, COMPARE> map_type;
    typedef std::pair<KEY,VALUE> value_type;
    typedef KEY key_type;
    typedef VALUE mapped_type;
    typedef vector_map_iterator<vector_map> iterator;
    typedef vector_map_iterator<vector_map> const_iterator;
    typedef typename iterator::difference_type difference_type;

    vector_map () : m_isDirty (false) {}
    
    vector_map& insert (const value_type& val)
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
        std::for_each (first, last, [&](const typename InputIterator::value_type& val)
                       {
                           m_keys.push_back (val.first);
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

    

    iterator begin () { cleanUp (); return vector_map_iterator<vector_map> (*this, ITERATOR_TYPE::BEGIN); }
    iterator end   () { cleanUp (); return vector_map_iterator<vector_map> (*this, ITERATOR_TYPE::END  ); }
    const_iterator begin () const { cleanUp (); return vector_map_iterator<vector_map> (*this, ITERATOR_TYPE::BEGIN); }
    const_iterator end   () const { cleanUp (); return vector_map_iterator<vector_map> (*this, ITERATOR_TYPE::END  ); }
    
private:

    void cleanUp () const
    {
        if (!m_isDirty)
            return;
        
        // apply the permutation
        std::vector<size_t> permutation = sort_permutation<key_type, COMPARE> (m_keys);
        m_keys = apply_permutation (m_keys, permutation);
        m_values = apply_permutation (m_values, permutation);
        
        m_isDirty = false;
    }
    
    mutable std::vector<key_type> m_keys;
    mutable std::vector<mapped_type> m_values;

    mutable bool m_isDirty;


    friend class vector_map_iterator<vector_map>;
};




template <typename map_type>
void probe (map_type& testMap, std::string name, auto keys, auto values, size_t toFind)
{
    // map_type
    const int width (15);
    std::cout << std::setw (width) << keys.size () << " : " << std::setw (width) << name << " : " << std::flush;
    high_resolution_clock::time_point startTime = high_resolution_clock::now ();
    auto itValues = begin (values);
    for_each (begin (keys), end (keys), [&testMap, &itValues](size_t key)
              {
                  testMap.insert (std::make_pair (key, (*itValues++)));
              });

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
    duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
    std::cout << "time = " << time_span.count () << std::setw (width) << "   found = " << std::setw (width) << currFind << std::endl;
}



int main()
{
    // typedef vector_map<size_t,std::string> VecMapType;
    // VecMapType vecMap;
    // vecMap.insert (std::make_pair (3, std::string ("hallo")));
    // vecMap.insert (std::make_pair (1, std::string ("test")));
    // vecMap.insert (std::make_pair (4, std::string ("wir")));

    // for (auto it = begin (vecMap), itEnd = end (vecMap); it != itEnd; ++it)
    // {
    //     const auto& entry (*it);
    //     std::cout << "key = " << entry.first << "  val = " << entry.second << std::endl;
    // }

    // VecMapType::iterator itFind = vecMap.find (3);
    // std::cout << "found = " << itFind->first << " with value " << itFind->second << std::endl;

    // vecMap.erase (vecMap.begin () + 1);
    
    // for (auto it = begin (vecMap), itEnd = end (vecMap); it != itEnd; ++it)
    // {
    //     const auto& entry (*it);
    //     std::cout << "key = " << entry.first << "  val = " << entry.second << std::endl;
    // }

    
    
    // return 0;
    
    
    
    std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000});
    std::vector<double> times0;
    std::vector<double> times1;

    typedef size_t key_type;
    typedef std::vector<size_t> value_type;
    
    size_t toFind = 10000;
    for (auto count : counts)
    {
        std::vector<key_type> keys (count);
        impl_iota (begin (keys), end (keys), 0);
        std::vector<value_type> values (count, value_type ({2,3}));
//        impl_iota (begin (values), end (values), 0);
        
        vector_map<key_type, value_type> testVecMap;
        probe (testVecMap, "vector_map", keys, values, toFind);

        // std::map<key_type, value_type> testMap;
        // probe (testMap, "std::map", keys, values, toFind);

    }

}

