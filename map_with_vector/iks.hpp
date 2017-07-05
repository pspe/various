#include <vector>
#include <iterator>

#pragma once





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

    template <typename T, typename comparison_type = std::less<T> >
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
            std::sort (m_data.begin (), m_data.end (), comparison_type ());
            m_data.erase (std::unique (m_data.begin (), m_data.end ()), m_data.end ());
            m_isDirty = false;
        }

    private:
        mutable std::vector<T>  m_data;
        mutable bool            m_isDirty;
    };

} // namespace IKS

