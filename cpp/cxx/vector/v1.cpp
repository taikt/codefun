// Example program
#if 0
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
using namespace std;

int main()
{
    // vector
    vector<int> first;// (4,100);
    first.assign(7,10); // assign 7 first elements with value of 10
    // first = 10 10 10 10 10 10 10

    for (int i = 1; i<4; i++)
    	first.push_back(i);
    // first = 10 10 10 10 10 10 10 1 2 3

    for (vector<int>::iterator it = first.begin(); it != first.end(); ++it)
    	cout <<' '<<*it;

    cout<<endl;

}
#endif
#include<bits/stdc++.h>
using namespace std;
int main(){
    vector<int> v;
    v.push_back(1);//Insert element 1.
    v.push_back(2);//Insert element 2.

    //Now the vector(v) has 2 elements in it, with size 2

    v.pop_front(); // This method will remove the first element

    for(int i=0;i<v.size();i++){
        cout << v[i] << " ";
    }
    //Prints [1]
    return 0;
}
