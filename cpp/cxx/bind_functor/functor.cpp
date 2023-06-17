#include <iostream>
#include <string>
using namespace std;

// day la functor
// functor override operator()
// functor la object nhung hoat dong nhu function
class MatchString {
public:    
    bool operator()(string& str) {
        return str=="lion";
    }
};

int main() {
    MatchString matcher;
    string input="lions";

    //matcher la 1 object nhung hoat dong nhu call fuction ==> goi la functor
    if (matcher(input))
        cout<<"matching\n";
    else 
        cout<<"unmatching\n";

    return 0;
}