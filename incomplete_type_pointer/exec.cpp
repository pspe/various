#include <iostream>
#include "test.hpp"



int main ()
{
    const A* a = get ();
    doAction (a);
    discard (a);
}



