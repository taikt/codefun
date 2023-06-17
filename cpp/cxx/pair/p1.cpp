// Example program
#include <iostream> //std::cout
#include <utility> // std::pair, std::make_pair
#include <string> //std::string

using namespace std;
int main()
{
    pair<string, double> product1("hello",3.4);
    
    product1.first = "kaka";
    product1.second = 4.6;
    
    cout <<"price of "<<product1.first<<" is "<<product1.second<<endl;
    
    
    return 0;
}
