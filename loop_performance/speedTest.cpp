
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <chrono>
#include <random>
#include <fenv.h>






template <typename T>
T gaussDouble (T mean, T sigma)
{
    static std::default_random_engine generator;
    std::normal_distribution<T> distribution (mean, sigma);
    return distribution (generator);
}


template <typename T>
T uniformDouble (T minValue, T maxValue)
{
    static std::default_random_engine generator;
    std::uniform_real_distribution<T> distribution(minValue, maxValue);
    return distribution(generator);
}


    
template <typename T>
T studenttDouble (T distributionParameter)
{
    static std::default_random_engine generator;
    std::student_t_distribution<T> distribution (distributionParameter);
    return distribution (generator);
}

    
/* DNNTYPE gaussDouble (DNNTYPE mean, DNNTYPE sigma); */
/* DNNTYPE studenttDouble (DNNTYPE distributionParameter); */
int randomInt (int maxValue);
/* DNNTYPE uniformDouble (DNNTYPE minValue, DNNTYPE maxValue); */
    
template <typename Container, typename T>
void uniform (Container& container, T maxValue);

template <typename Container, typename T>
void gaussDistribution (Container& container, T mean, T sigma);



template <typename Container, typename T>
void uniform (Container& container, T maxValue)
{
    for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
    {
        (*it) = uniformDouble (-1.0*maxValue, 1.0*maxValue);
    }
}


template <typename Container, typename T>
void gaussDistribution (Container& container, T mean, T sigma)
{
    for (auto it = begin (container), itEnd = end (container); it != itEnd; ++it)
    {
        (*it) = gaussDouble (mean, sigma);
    }
}



    
    

template <typename ItSource, typename ItWeight, typename ItTarget>
void applyWeights_sub (ItSource itSourceBegin, ItSource itSourceEnd,
                       ItWeight itWeight,
                       ItTarget itTargetBegin, ItTarget itTargetEnd)
{
    // std::vector<typename std::iterator_traits<ItTarget>::value_type> tmp;
    // size_t numTargets = std::distance (itTargetBegin, itTargetEnd);
    // tmp.resize (numTargets);
    // for_each (itSourceBegin, itSourceEnd, [itTargetBegin, itTargetEnd, &itWeight, &tmp, numTargets](auto source)
    //           {
    //               for (size_t i = 0; i < numTargets; ++i, ++itWeight)
    //                   tmp.at (i) = source * (*itWeight);
    //               auto itTmp = begin (tmp);
    //               for_each (itTargetBegin, itTargetEnd, [&itTmp](auto& target)
    //                         {
    //                             target += (*itTmp);
    //                             ++itTmp;
    //                         });
    //           });


    // for_each (itTargetBegin, itTargetEnd, [itSourceBegin, itSourceEnd, &itWeight](auto& target)
    //           {
    //               for_each (itSourceBegin, itSourceEnd, [&itWeight, &target](auto source)
    //                         {
    //                             target += source * (*itWeight);
    //                             ++itWeight;
    //                         });
    //           });


    for_each (itSourceBegin, itSourceEnd, [itTargetBegin, itTargetEnd, &itWeight](auto source)
              {
                  for_each (itTargetBegin, itTargetEnd, [&itWeight, source](auto& target)
                            {
                                target += source * (*itWeight);
                                ++itWeight;
                            });
              });
}


    
/*! \brief apply weights using drop-out; for no drop out, provide (&bool = true) to itDrop such that *itDrop becomes "true"
 *
 * itDrop correlates with itSourceBegin 
 */
template <typename ItSource, typename ItWeight, typename ItTarget, typename ItDrop>
void applyWeights_sub (ItSource itSourceBegin, ItSource itSourceEnd,
                       ItWeight itWeight,
                       ItTarget itTargetBegin, ItTarget itTargetEnd,
                       ItDrop itDrop)
{
    size_t dist = std::distance (itSourceBegin, itSourceEnd);
    for_each (itTargetBegin, itTargetEnd, [itSourceBegin, itSourceEnd, &itDrop, &itWeight, dist](auto& target)
              {
                  if (!(*itDrop))
                  {
                      itWeight += dist;
                      return;
                  }

                  typename std::iterator_traits<ItTarget>::value_type tmp (0.0);
                  for_each (itSourceBegin, itSourceEnd, [&itWeight, &tmp](auto source)
                            {
                                tmp += source * (*itWeight);
                                ++itWeight;
                            });
                  target += tmp;
              });


    // for (auto itTarget = itTargetBegin; itTarget != itTargetEnd; ++itTarget)
    // {
    //     if (*itDrop)
    //     {
    //         typename std::iterator_traits<ItTarget>::value_type tmp (0.0);
    //         for (auto itSource = itSourceBegin; itSource != itSourceEnd; ++itSource, ++itWeight)
    //         {
    //             tmp += *itSource * (*itWeight);
    //         } 
    //         *itTarget += tmp;
    //     }
    //     else
    //         itWeight += std::distance (itSourceBegin, itSourceEnd);
                    
                    
    //     ++itDrop;        
    // }
}



template <bool HasDropOut, typename ItSource, typename ItWeight, typename ItTarget, typename ItDrop>
void applyWeights (ItSource itSourceBegin, ItSource itSourceEnd,
                   ItWeight itWeight,
                   ItTarget itTargetBegin, ItTarget itTargetEnd,
                   ItDrop itDrop)
{
    if (HasDropOut)
        applyWeights_sub (itSourceBegin, itSourceEnd, itWeight, itTargetBegin, itTargetEnd, itDrop);
    else
        applyWeights_sub (itSourceBegin, itSourceEnd, itWeight, itTargetBegin, itTargetEnd);
}





template <typename TESTTYPE>
void test_applyWeights (int version, int numSource, int numTarget, int numDropVariants, float dropCut, int iTimes = 5)
{
    std::vector<TESTTYPE> source;
    __thread std::vector<TESTTYPE> weights;
    std::vector<TESTTYPE> target; 
    std::vector<TESTTYPE> target_1; 
    std::vector<std::vector<char>> dropVariants;
    dropVariants.resize (numDropVariants);
    std::vector<float> dropProbability; 
    source.resize (numSource);
    target.assign (numTarget, 0.0);
    target_1.assign (numTarget, 0.0);
    weights.resize (source.size () * target.size ());
    gaussDistribution (weights, 0.1, 0.4);
    gaussDistribution (source, 0.1, 0.4);

    dropProbability.resize (target.size ());
    for (auto itDropVariant = begin (dropVariants), itDropVariantEnd = end (dropVariants); itDropVariant != itDropVariantEnd; ++itDropVariant)
    {
        std::vector<char>& drop = *itDropVariant;
        drop.resize (target.size ());
        uniform (dropProbability, 1.0);
        auto itDrop = begin (drop);
        for (auto it = begin (dropProbability), itEnd = end (dropProbability); it != itEnd; ++it, ++itDrop)
        {
            *itDrop = *it <= dropCut;
        }
        // std::cout << "proba" << std::endl;
        // std::copy (begin (dropProbability), end (dropProbability), std::ostream_iterator<float>(std::cout, " "));
        // std::copy (begin (drop), end (drop), std::ostream_iterator<char>(std::cout, " "));
        // std::cout << std::endl;
    }

   

    
    std::chrono::time_point<std::chrono::system_clock> start, stop;
    start = std::chrono::system_clock::now ();
    for (int i = 0; i < iTimes; ++i)
    {
        for (auto drop : dropVariants)
        {
            if (version == 0)
                applyWeights<false> (begin (source), end (source), begin (weights), begin (target), end (target), begin (drop));
            if (version == 1)
                applyWeights<true> (begin (source), end (source), begin (weights), begin (target), end (target), begin (drop));
        }
    }
    stop = std::chrono::system_clock::now ();
    std::chrono::duration<double> elapsed = stop-start;
    std::cout << "version = " << version
              << "  type = " << typeid (TESTTYPE).name ()
              << "  source = " << source.size ()
              << "  target = " << target.size ()
              << "  dropVariants = " << dropVariants.size ()
              << "  drop-cut = " << dropCut
              << "  elapsed time = " << elapsed.count ()
              << std::endl;
}



int main ()
{ 
//    return 1;
//    feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW|FE_UNDERFLOW);
    feenableexcept (FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW); // exceptions bei underflow, overflow und divide by zero (damit man den fehler gleich findet)


    for (int iVersion = 0; iVersion < 2; ++iVersion)
    {
        // test_applyWeights<float> (10, 10, 1, 0.0, 1);

        test_applyWeights<float> (iVersion, 1000, 1000, 10, 0.5, 100);
        test_applyWeights<double> (iVersion, 1000, 1000, 10, 0.5, 100);
    }
    return 0;
} 

