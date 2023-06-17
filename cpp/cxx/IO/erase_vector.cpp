#include <bits/stdc++.h>
using namespace std;

int main() {

    vector<int> v={1,2,3,4};
    auto it=find(v.begin(),v.end(),2);
    //v.erase(++it,v.end()); // v={1,2}
    v.erase(v.begin(),++it);//v={3,4}
    // rule: v.erase(begin,last)
    // last point to po_+1 (po_ is position of found value)
    for(auto x:v)
        cout<<x<<" ";

    return 0;
}
