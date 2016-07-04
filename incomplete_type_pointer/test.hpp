#include <iostream>


class A;

const A* get ();
void discard (const A* a);




void doAction (const A* a)
{
    std::cout << "out : " << a->value () << std::end;
}

