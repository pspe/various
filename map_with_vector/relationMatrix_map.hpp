#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>
#include <iterator>
#include <map>

#include "iks.hpp"


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

        if (!m_leftSideComplete && (void*)&data == (void*)&m_leftPrimaryElements)
        {
            cleanUp (m_rightPrimaryElements);
            crossBuild (m_rightPrimaryElements, m_leftPrimaryElements);
        }
        else if (!m_rightSideComplete &&
		 (void*)&data == (void*)&m_rightPrimaryElements)
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



