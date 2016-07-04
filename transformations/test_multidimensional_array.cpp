
#include <iostream>
#include <algorithm>
#include <iterator>


int main ()
{
    int arr[3][3];
    int count = 0;
    for (int i=0; i<3; ++i)
	for (int j=0; j<3; ++j)
	    arr[i][j] = count++;

    std::copy (*arr, *arr +9, std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    // writes out: 0 1 2 3 4 5 6 7 8 
}
