#include <iostream>

int main()
{
	double a = 0;
	double b = 0;
	
	double c = a/b;

	std::cout << "c==c  " << (c==c ? "true" : "false") << std::endl;
	std::cout << "c!=c  " << (c!=c ? "true" : "false") << std::endl;
	std::cout << "c>1.0  " << (c>1.0 ? "true" : "false") << std::endl;
	std::cout << "c<1.0  " << (c<1.0 ? "true" : "false") << std::endl;
	std::cout << "c==1.0 " << (c==1.0 ? "true" : "false") << std::endl;

}

