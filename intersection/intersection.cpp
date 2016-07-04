#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

using namespace std::chrono;

template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator base_intersection (InputIterator1 first1, InputIterator1 last1,
                                  InputIterator2 first2, InputIterator2 last2,
                                  OutputIterator result)
{
    while (first1!=last1 && first2!=last2)
    {
        if (*first1<*first2) ++first1;
        else if (*first2<*first1) ++first2;
        else {
            *result = *first1;
            ++result; ++first1; ++first2;
        }
    }
    return result;
}


template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator rnd_intersection (InputIterator1 first1, InputIterator1 last1,
                                 InputIterator2 first2, InputIterator2 last2,
                                 OutputIterator result)
{
    size_t len1 = std::distance (first1, last1);
    size_t len2 = std::distance (first2, last2);

    if (len1 < len2)
        return rnd_intersection (first2, last2, first1, last1, result);

    if (len2 == 0)
        return result;

    if (len2 == 1)
    {
        if (std::binary_search (first1, last1, *first2))
        {
            *result = *first2;
            ++result;
        }
        return result;
    }
    
    if (len1 < 10)
    {
        return std::set_intersection (first1, last1, first2, last2, result);
    }
    
    InputIterator2 itMiddle2 = first2 + len2/2;
    InputIterator1 itLoc1 = std::lower_bound (first1, last1, *itMiddle2);
    InputIterator1 itLoc1Up = std::upper_bound (first1, last1, *itMiddle2);

    rnd_intersection (first1, itLoc1, first2, itMiddle2, result);
    rnd_intersection (itLoc1Up, last1, itMiddle2, last2, result);
    return result;
}






int main ()
{
    
    std::vector<int> listA (1000);
    std::vector<int> listB (500);
    std::iota (std::begin (listA), std::end (listA), 0);
    std::iota (std::begin (listB), std::end (listB), 300);


    {
        std::vector<int> intersection;
        high_resolution_clock::time_point startTime = high_resolution_clock::now ();

        std::set_intersection (listA.begin (), listA.end (), listB.begin (), listB.end (), std::back_inserter (intersection));

        high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
        duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
        std::cout << "std intersection : " << intersection.size () << " with time : " << time_span.count () << std::endl;
    }
    {
        std::vector<int> intersection;
        high_resolution_clock::time_point startTime = high_resolution_clock::now ();

        base_intersection (listA.begin (), listA.end (), listB.begin (), listB.end (), std::back_inserter (intersection));

        high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
        duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
        std::cout << "base intersection : " << intersection.size () << " with time : " << time_span.count () << std::endl;
        //for (int& i:intersection) std::cout << ' ' << i;
    }
    {
        std::vector<int> intersection;
        high_resolution_clock::time_point startTime = high_resolution_clock::now ();

        rnd_intersection (listA.begin (), listA.end (), listB.begin (), listB.end (), std::back_inserter (intersection));

        high_resolution_clock::time_point endTime   = high_resolution_clock::now ();
        duration<double> time_span = duration_cast<duration<double>> (endTime-startTime);
        std::cout << "rnd intersection : " << intersection.size () << " with time : " << time_span.count () << std::endl;
        //for (int& i:intersection) std::cout << ' ' << i;
    }
    
}
