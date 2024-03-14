#include <bits/stdc++.h>
using namespace std;
multiset<int,greater<int>> s;
// 4 3 3 2 1
int n;
int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    for (int i=0;i<n;i++) {
        int a;
        cin>>a;
        s.insert(a); // O(logn)
    }
    for (auto &a: s){
        cout<<a<<" ";
    }
    cout<<"\n";
    return 0;
}