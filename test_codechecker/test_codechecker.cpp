#include <iostream>
//#include <iomanip>
//#include <vector>
//#include <algorithm>
//#include <iterator>
//#include <functional>
//#include <set>
//#include <chrono>
#include <cassert>
#include "simple.hpp"



//using namespace std::chrono;



void f(int* p) {}

int doSomething (int val)
{
//    std::cout << val << std::endl;
    return val + 3;
}

int* const MyNullPointer = NULL;

// #define TEST_CRASH(err) assert(MyNullPointer); *MyNullPointer = err; 
#ifndef __clang_analyzer__
#define TEST0_CRASH(err) *MyNullPointer = err;
#else
#define TEST0_CRASH(err) int r = err;
#endif

#ifndef __clang_analyzer__
#define TEST1_CRASH(err) *MyNullPointer = err;
#else
#define TEST1_CRASH(err) exit(err);
#endif

#define TEST2_CRASH(err) *MyNullPointer = err;




size_t myfnc ()
{
    size_t x = 5;
    return x*2;
}


void crash0 () 
{
    int* doom (0);
    *(doom) = 11; 
}

void crash1 () __attribute__((analyzer_noreturn));
void crash1 () 
{
    int* doom (0);
    #ifndef __clang_analyzer__
    *(doom) = 11; 
    #endif
}

void crash2 () 
{
    exit (13);
}


void customAssert0 (); // to turn off the error message below customAssert: __attribute__((analyzer_noreturn));

void customAssert0 ()
{
}

void customAssert1 () __attribute__((analyzer_noreturn));

void customAssert1 ()
{
}


int testNoReturn0 (int* b)
{
    if (!b)
        customAssert0 (); 
    return *b;
}

int testNoReturn1 (int* b)
{
    if (!b)
        customAssert1 (); 
    return *b;
}


struct Test
{
    int fnc () { return 3; }
};

int testNullPointerDereference (Test* t)
{
    // assert (t != 0); // add assert statement to exclude analyzation paths
    return t->fnc ();
}

int testDivByZero (int length)
{
    // assert (length > 0); // turn off div_by_zero error by asserting, that length > 0
    int x = 0;
    for (int i = 0; i < length; ++i)
        ++x;
    return length/x;
}



int main()
{

    {
        size_t val = 123;
        size_t x = val << 10;
        size_t y = val << 63;
        size_t z = val << 64;
        size_t a = val << 65;
    }
    
    {
        size_t y = myfnc ();
    }
    
    {
        Test* t (0);
        if (!t)
//            TEST0_CRASH(24)
//            TEST1_CRASH(24)
            TEST2_CRASH(24)
        t->fnc ();
    }





    {
        Test* t (0);
        if (!t)
            crash2 ();
        t->fnc ();
    }

    {
        Test* t (0);
        if (!t)
            crash1 ();
        t->fnc ();
    }
    
    {
        Test* t (0);
        if (!t)
            crash2 ();
        t->fnc ();
    }

    
    {
        int ret = testNullPointerDereference (0);
    }

  
    {
        int test;
        test = doSomething (test);
    }

    {
        int x;
        if (x) // NOLINT // warn
            return 1;
    }

    {
        static int *x;
        int y;
        x = &y; // warn
    }

    {
        int* test_nullptr_deref = 0;
        *test_nullptr_deref = 5;
    }

    // while (true)
    // {
    //     std::cout << "hallo" << std::endl;
    // }

    int arr[5];
    arr[5] = 3;
    std::cout << "var = " << arr[5] << std::endl;

    {
        int* test_doubledelete = new int (23);
        delete test_doubledelete;
        delete test_doubledelete;
        f (test_doubledelete);
    }

    {
        int i, a[10];
        // --- codechecker_suppress [deadcode.DeadStores] suppress deadcode
        int x = a[i]; // NOLINT // warn: array subscript is undefined
    }


    
    {
        int x;
        x |= 1; // NOLINT // warn: left expression is unitialized
    }

    {
        int x;
        x |= 2; // NOLINT // warn: left expression is unitialized
    }

    {
      int v (getValue ());
    }
    
}

