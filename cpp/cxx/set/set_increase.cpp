#include <bits/stdc++.h>
using namespace std;
set<int> s;
// 1 2 3 4
int n;
int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    for (int i=0;i<n;i++) {
        int a;
        cin>>a;
        s.insert(a);
    }
    for (auto &a: s){
        cout<<a<<" ";
    }
    cout<<"\n";
    return 0;
}