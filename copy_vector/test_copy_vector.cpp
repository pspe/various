#include <iostream>
#include <vector>

int main ()
{
    std::vector<size_t> v;
    v.push_back (3);
    v.push_back (5);

    std::vector<size_t> v_copy (v);
    std::cout << "copy: ";
    for (std::vector<size_t>::const_iterator it = v_copy.begin(), itEnd = v_copy.end(); it != itEnd; ++it)
    {
	std::cout << " " << (*it);
    }
    std::cout << std::endl;
}

