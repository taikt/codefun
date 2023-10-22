#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;



int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    string line;
    getline(cin, line);
    stringstream str(line);
    int num;
    vector<int> a;
    while (str>>num) {
        a.push_back(num);
    }
    int x;
    cin>>x;

    cout<<a;

    sort(a.begin(), a.end()); // increasing
    cout<<a;
    
    if (binary_search(a.begin(), a.end(), x))
    cout<<"found\n";
    else cout<<"not found\n";


    /*
    // find upper bound not using lib: 
    // uppper bound position: lo
    int lo = 0, hi = n - 1;
    int mid;
    while (lo < hi) {
        mid = (lo+hi)/2;
        if (x < a[mid]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    cout<<"lo="<<lo<<", value:"<<a[lo]<<"\n";
    */

    // if x is in array or even not, return position of next higher one
    cout<< "upper bound position of "<<x<<" is: "
    <<upper_bound(a.begin(), a.end(), x) - a.begin()<<"\n";
    
    // if x is in array, return position of x
    // otherwise, position of next higher one
    cout<< "lower bound position of "<<x<<" is: "
    <<lower_bound(a.begin(), a.end(), x) - a.begin() <<"\n";
    

    return 0;
}