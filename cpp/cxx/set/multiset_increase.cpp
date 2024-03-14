#include <bits/stdc++.h>
using namespace std;
multiset<int> s;
// 1 2 3 3 4
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
    cout<<"end element:"<<*s.rbegin()<<"\n";
    cout<<"begin element:"<<*s.begin()<<"\n";
    return 0;
}