#include <iostream>

class Test
{
    public:
        void compare (const int& param = EMPTY)
        {
            if (&param == &EMPTY)
                std::cout << "param is EMPTY" << std::endl;  
            if (param == EMPTY)
                std::cout << "param is default value" << std::endl;

            std::cout << "param == " << param << std::endl;
            std::cout << " === " << std::endl;
        }

        static const int EMPTY;
};

const int Test::EMPTY = 6;


int main ()
{
    Test test;
    test.compare ();
    test.compare (6);
    test.compare (4);
}

