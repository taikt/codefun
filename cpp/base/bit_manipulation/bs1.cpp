// bitset::test
#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>        // std::size_t
#include <bitset>         // std::bitset
using namespace std;

int main ()
{
    std::bitset<5> foo (std::string("01011"));

    std::cout << "foo contains:\n";
    //std::cout << std::boolalpha;
    for (std::size_t i=0; i<foo.size(); ++i)
       std::cout << foo.test(i) << '\n';
 
	// bitset
	bitset<4> foo2(string("1011"));

	cout <<foo2<<endl;
	cout <<foo2.reset(3)<<endl;
	//cout <<foo.reset()<<endl;
	//cout <<foo.set()<<endl;
	cout <<foo2.set(3)<<endl;

  return 0;
}