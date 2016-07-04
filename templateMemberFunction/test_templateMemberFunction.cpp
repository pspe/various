
#include <iostream>



enum SIDE { LEFT = 0, RIGHT = 1 };

template <SIDE Side>
struct Other
{
    static const SIDE side = LEFT;
};

template <>
struct Other<LEFT>
{
    static const SIDE side = RIGHT;
};



struct Test
{

    template <SIDE side>
    SIDE value () const;
};


template <>
SIDE Test::value<LEFT> () const { return LEFT; }

template <>
SIDE Test::value<RIGHT> () const { return RIGHT; }



int main ()
{

    Test t;
    
    std::cout << "t.value<LEFT> ()  = " << t.value<LEFT> () << std::endl;
    std::cout << "t.value<RIGHT> () = " << t.value<RIGHT> () << std::endl;
    std::cout << "Other<LEFT>::side  = " << Other<LEFT>::side << std::endl;
    std::cout << "Other<RIGHT>::side = " << Other<RIGHT>::side << std::endl;
    std::cout << "t.value<Other<LEFT>::side> ()  = " << t.value<Other<LEFT>::side> () << std::endl;
    std::cout << "t.value<Other<RIGHT>::side> () = " << t.value<Other<RIGHT>::side> () << std::endl;

    const SIDE otherSide (Other<LEFT>::side);
    std::cout << "t.value<otherSide> () = " << t.value<otherSide> () << std::endl;
}

