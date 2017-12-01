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
#include <queue>

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



template <typename T>
class Hundred
{
public:

    typedef T value_type;
    
    
    Hundred () : m_currentSize (0)
    {
        m_data.reserve (101);
    }
    
    void insert (const T& value)
    {
        ++m_currentSize;
        if (m_currentSize < 100)
        {
            m_data.push_back (value);
            return;
        }

        if (m_currentSize == 100)
        {
            m_data.push_back (value);
            std::sort (m_data.begin (), m_data.end ());
            return;
        }

        if (value <= m_data.front ())
            return;

        m_data.erase (m_data.begin ());
        typename std::vector<T>::const_iterator it = std::lower_bound (m_data.begin (), m_data.end (), value);
        m_data.insert (it, value);
    }

    typename std::vector<T>::iterator begin () { return m_data.begin (); }
    typename std::vector<T>::iterator end   () { return m_data.end   (); }
    
private:

    std::vector<T> m_data;
    size_t m_currentSize;
};


template <typename T>
class Hundred2
{
public:

    typedef T value_type;
    typedef std::vector<T> container_type;
    
    Hundred2 () : m_currentSize (0)
    {
        m_data.reserve (101);
    }
    
    void insert (const T& value)
    {
        ++m_currentSize;
        if (m_currentSize < 100)
        {
            m_data.push_back (value);
            return;
        }

        if (m_currentSize == 100)
        {
            m_data.push_back (value);
            std::sort (m_data.begin (), m_data.end (), std::greater<T>());
            return;
        }

        if (value <= m_data.back ())
            return;

        m_data.pop_back ();

        typename container_type::reverse_iterator it = std::lower_bound (m_data.rbegin (), m_data.rend (), value);
        size_t dist = 99 - std::distance (m_data.rbegin (), it);
        if (dist >= 99)
            m_data.push_back (value);
        else
            m_data.insert (m_data.begin () + dist, value);
    }

    typename container_type::iterator begin () { return m_data.begin (); }
    typename container_type::iterator end   () { return m_data.end   (); }
    
private:

    std::vector<T> m_data;
    size_t m_currentSize;
};



template <typename T>
class Hundred3
{
public:

    typedef T value_type;
    
    
    Hundred3 () : m_currentSize (0)
    {
        m_data.resize (101);
    }
    
    void insert (const T& value)
    { 
        if (m_currentSize < 100)
        {
            m_data[m_currentSize++] = value;
            return;
        }

        if (m_currentSize++ > 100)
            m_data[100] = value;
        std::sort (m_data.begin (), m_data.end ());
    }

    typename std::vector<T>::iterator begin () { return m_data.begin (); }
    typename std::vector<T>::iterator end   () { return m_data.end   (); }
    
private:

    std::vector<T> m_data;
    size_t m_currentSize;
};




template <typename CONTAINER>
void probe (CONTAINER& container, std::string name, std::vector<typename CONTAINER::value_type> vec)
{
    // map_type
    const int width (12);
    const int precision (6);
    std::cout << std::setw (width) << vec.size () << " : " << std::setw (width) << name << " : " << std::flush;

    high_resolution_clock::time_point startTime = high_resolution_clock::now ();

    for (const auto& value : vec)
    {
        container.insert (value);
    }
    
    high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
    duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
    std::cout << " t = " << std::setprecision (precision) << time_span.count () << std::endl;
}



int main()
{
    std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000, (size_t)1e8});
//    std::vector<size_t> counts ({1000});
//    std::vector<size_t> counts ({20});

    double randFactor = 100.0;
    for (auto count : counts)
    {
        size_t maxRand0 = size_t (count * randFactor);
        std::vector<size_t> values (count, 0);
        uniform (values, size_t(0), maxRand0);

        // std::cout << "--- values ---" << std::endl;
        // auto sorted = values;
        // std::sort (sorted.begin (), sorted.end (), std::greater<size_t>());
        // sorted.resize (100);
        // std::copy (sorted.rbegin (), sorted.rend (), std::ostream_iterator<size_t>(std::cout, ", "));

        
        std::cout << std::endl;
        Hundred<size_t> hundred;
        probe (hundred, "version 1 ", values);
        // std::cout << std::endl;
        // Hundred2<size_t> hundred2;
        // probe (hundred2, "version 2 ", values);
        std::cout << std::endl;
        Hundred3<size_t> hundred3;
        probe (hundred3, "version 3 ", values);

        // std::cout << "--- output ---" << std::endl;
        // std::copy (hundred.begin (), hundred.end (), std::ostream_iterator<size_t>(std::cout, ", "));
        // std::cout << std::endl;
    }
    return 0;

}

