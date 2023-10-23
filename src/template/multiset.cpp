#include <bits/stdc++.h>
using namespace std;

// https://www.geeksforgeeks.org/multiset-lower_bound-in-cpp-stl-with-examples/
// https://www.geeksforgeeks.org/multiset-upper_bound-in-cpp-stl-with-examples/?ref=lbp
// https://www.geeksforgeeks.org/multiset-in-cpp-stl/

int main() {
    freopen("input.txt","r",stdin);
    // input
    // 4
    // 12 4 67 8 12
    int n,x,u;
    cin >> n >> u;
    //multiset<int,greater<int>> m; // decreasing order
    multiset<int> m; // increasing order if printed
    for (int i = 0; i < n; i++) {
        cin >> x;
        m.insert(x);
    }

    for (auto el: m) {
        cout<<el<<" ";
    }
    cout<<"\n";

    auto it = m.upper_bound(u); // return position to next one that greater than u
    if (it == m.begin()) {
        cout<<"return begin of multiset\n"; // if u is smaller than every element in set
    } else if (it == m.end()) {
        cout<<"return end of multiset, previous element:"<<*(--it)<<"\n"; // // if u is larger than every element in set
    }
    cout<<"find upper bound of "<<u<<" is:"<<*(m.begin())<<"\n";

    /*
    auto it2 = m.lower_bound(u); // return position to one equal to u (if u exist on set), 
                                 // or next one that greater than u (if u not exist on set )
    cout<<"find lower bound of "<<u<<" is:"<<*it2<<"\n";
    */
    return 0;
}