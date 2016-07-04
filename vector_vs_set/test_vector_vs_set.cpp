#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <set>
#include <chrono>
#include <cassert>



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



int main()
{
    std::vector<size_t> counts ({100, 1000, 10000, 100000, 1000000, 10000000});
    std::vector<double> times_vector;
    std::vector<double> times_set;

    size_t maxRand = 100000000;
//    size_t maxRand = 100;
    size_t toFind = 10000;
    for (auto count : counts)
    {
        std::cout << "performing " << count << std::endl;
        std::vector<size_t> values (count, 0);
        uniform (values, size_t(0), maxRand);
        
        // vector
        std::cout << "... vector ... " << count << std::endl;
        high_resolution_clock::time_point startTime = high_resolution_clock::now ();
        std::vector<size_t> testVector;
        for_each (begin (values), end (values), [&testVector](size_t v) { testVector.push_back (v); });
        std::sort (begin (testVector), end (testVector));
        testVector.erase (std::unique (begin (testVector), end (testVector)), end (testVector));

        size_t currFind = 0;
        for (auto v : values)
        {
            if (currFind >= toFind)
                break;
            //bool exists = std::binary_search (begin (testVector), end (testVector), v);
            bool exists = std::lower_bound (begin (testVector), end (testVector), v) != end (testVector);
            if (exists)
                ++currFind;
        }
        high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
        duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
        times_vector.push_back (time_span.count ());
        std::cout << "    found = " << currFind << std::endl;

        // set
        std::cout << "... set    ... " << count << std::endl;
        startTime = high_resolution_clock::now ();
        std::set<size_t> testSet;
        for_each (begin (values), end (values), [&testSet](size_t v) { testSet.insert (v); });

        currFind = 0;
        for (auto v : values)
        {
            if (currFind >= toFind)
                break;
            bool exists = testSet.find (v) != testSet.end ();
            if (exists)
                ++currFind;
        }

        endTime   = high_resolution_clock::now ();
//        assert (testSet.size () == values.size ());
        time_span = duration_cast<duration<double>> (endTime-startTime);
        times_set.push_back (time_span.count ());
        std::cout << "    found = " << currFind << std::endl;

        assert (testVector.size () == testSet.size ());
        std::cout << "inserted = " << testVector.size () << std::endl;
    }

    auto itSize (begin (counts));
    auto itSizeEnd (end (counts));
    auto itVect (begin (times_vector));
    auto itSet (begin (times_set));

    const int width (15);
    std::cout << "#samples" << std::setw (width) << "vector" << std::setw (width) << "set" << std::endl;
    for (; itSize != itSizeEnd; ++itSize, ++itVect, ++itSet)
    {
        std::cout << std::setw (width) << (*itSize) << std::setw (width) << (*itVect) << std::setw (width) << (*itSet) << std::setw (width) << double(*itSize) << std::endl;
    }
}

