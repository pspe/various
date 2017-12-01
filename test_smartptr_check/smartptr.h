#ifndef  __SMARTPTR_H__
#define  __SMARTPTR_H__


#include <string>
#include <atomic>


namespace IKS
{


    
/*=================================================================*/
/*!
   \brief Any class used with a Ptr<T> has to be derived from 
   IRefCounter. Such a class can not be destroyed by delete!
*/
class IRefCounter
{
public:
    IRefCounter ()
	: m_ulRefCount (0)
    {}
    
   virtual long AddRef(void) 
   {
      return static_cast<long>(++m_ulRefCount);
   }

   virtual long Release(void) 
   {
      int  l(--m_ulRefCount);
      if (l == 0) 
         delete this;
      return static_cast<long>(l);
   }

   virtual long AddRef(void) const
   {
      return static_cast<long>(++m_ulRefCount);
   }

   virtual long Release(void) const
   {
      int  l(--m_ulRefCount);
      if (l == 0) 
         delete this;
      return static_cast<long>(l);
   }

protected:
   long  LockForConstruction(void)
   {
      return static_cast<long>(++m_ulRefCount);
   }

   long  UnlockForConstruction(void)
   {
      int  l(m_ulRefCount);
      if(l > 0)
         l = --m_ulRefCount;
      return static_cast<long>(l);
   }

   long  LockForConstruction(void) const
   {
      return static_cast<long>(++m_ulRefCount);
   }

   long  UnlockForConstruction(void) const
   {
      int l(m_ulRefCount);
      if(l > 0)
         l = --m_ulRefCount;
      return static_cast<long>(l);
   }


protected:
   virtual ~IRefCounter(void) {}

private:
   mutable std::atomic<long>  m_ulRefCount;
};


/*=================================================================*/
/*!
   \brief The class Ptr<T> implements a smart pointer for a class of
   type T that is derived from IRefCounter. The defined operators
   make the class look like a T* but implements reference counting.
*/
template <class T>
class Ptr
{
public:
   typedef T            value_type;
   typedef value_type*  pointer_type;

   Ptr(void) : m_pPtr(0) 
      {}
   Ptr(T* pPtr) : m_pPtr(pPtr) 
      { if (m_pPtr != 0) m_pPtr->AddRef(); }
   Ptr(const Ptr<T>& pPtr) : m_pPtr(pPtr.m_pPtr) 
      { if (m_pPtr != 0) m_pPtr->AddRef(); }

   ~Ptr(void)
      { if (m_pPtr != 0) m_pPtr->Release();  }

public:
   T* Get(void) const { return ((T*)m_pPtr); }

   operator T*(void) const 
      { return ((T*)m_pPtr); }
   
   T& operator*(void) const
   {
       if (m_pPtr == NULL) { throw std::string ("Dereferencing 0 pointer"); };
      return (*m_pPtr);
   }
   
   T** AddressOf()
   {
      if (m_pPtr != NULL) { throw std::string("Referencing non 0 pointer"); };
      return (&m_pPtr);
   }
   
   T* operator->(void) const
   {
      if (m_pPtr == NULL) { throw std::string("Dereferencing 0 pointer"); };
      return ((T*)m_pPtr);
   }
   
   T* operator =(T* pPtr) 
   { 
      if (pPtr != 0) 
         pPtr->AddRef(); 
      if (m_pPtr != 0) 
         m_pPtr->Release(); 
      m_pPtr = (T*)pPtr; 
      return (m_pPtr); 
   } 

   T* operator =(const Ptr<T>& ptrPtr) 
   { 
      if (ptrPtr.m_pPtr != 0) 
         ptrPtr.m_pPtr->AddRef(); 
      if (m_pPtr != 0) 
         m_pPtr->Release(); 
      m_pPtr = ptrPtr.m_pPtr; 
      return (m_pPtr); 
   } 


   // ---- move semantics -----
   Ptr(Ptr<T>&& pPtr)
       : m_pPtr (pPtr.m_pPtr)
   {
       pPtr.m_pPtr = NULL;
   }

   
  
   T* operator =(Ptr<T>&& ptrPtr)
   {
       std::swap (m_pPtr, ptrPtr.m_pPtr);
       return (m_pPtr);
   }
       
   
   bool operator!(void) const 
      { return (m_pPtr == NULL); }

   bool operator<(T* pT) const 
      { return (m_pPtr < pT); }

   bool operator==(T* pT) const 
      { return (m_pPtr == pT); }

protected:
   T*    m_pPtr;
};

}  // namespace ICF

#endif // __SMARTPTR_H__
