
#include <iostream>
#include <vector>

int main ()
{
    std::vector<size_t> myVec;
    myVec.push_back (3);
    myVec.push_back (4);
    myVec.push_back (5);
    myVec.push_back (6);
    myVec.push_back (7);

    std::vector<size_t>::iterator it = myVec.begin();
    std::vector<size_t>::const_iterator itC = myVec.begin();
    std::vector<size_t>::const_iterator itCEnd = myVec.end();

    for (; itC != itCEnd; ++itC)
    {
	std::cout << (*itC) << std::endl;
    }
}

