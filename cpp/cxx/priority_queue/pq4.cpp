#include<bits/stdc++.h>
using namespace std;
#define pq priority_queue
#define fi first
#define se second
#define pp pair<int,int>
#define l(i,a,b) for(int i=a;i<b;i++)
int main(){
    freopen("input.txt","r",stdin);
    int n;
    cin>>n;
    vector<int> a(n);
    for(auto &x:a) cin>>x;
    cout<<"before\n";
    for(auto &x:a) cout<<x<<" ";
    cout<<"\nafter queue\n";
    vector<pp> v;
    auto cmp=[=](int i, int j){
        if (a[i]>a[j]) return true;
        else return false;
    };
    pq<int,vector<int>,decltype(cmp)>q(cmp);
    l(i,0,n) q.push(i);
    while(!q.empty()){
        int i=q.top();q.pop();
        cout<<a[i]<<" ";
    }
    return 0;
}