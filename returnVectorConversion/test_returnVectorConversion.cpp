#include <algorithm>
#include <iostream>
#include <vector>




struct A
{
    operator const std::vector<int> () const
    {
        std::vector<int> v;
        v.push_back (1);
        v.push_back (2);
        v.push_back (3);
        return v;
    }
};



int main ()
{
    std::vector<int> v;
    v = A ();
    
    std::cout << "vec = ";
    for (std::vector<int>::const_iterator it = v.begin (), itEnd = v.end (); it != itEnd; ++it)
        std::cout << *it << " ";
    std::cout << std::endl;
}
