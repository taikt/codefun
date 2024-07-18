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
    vector<int> a(n),b(n);
    for(auto &x:a) cin>>x;
    for(auto &x:b) cin>>x;
    cout<<"before\n";
    for(auto &x:a) cout<<x<<" ";
    cout<<"\n";
    for(auto &x:b) cout<<x<<" ";
    cout<<"\nafter queue\n";
    vector<pp> v;
    l(i,0,n) v.push_back({a[i],b[i]});
    // order [i j] that a[i]<a[j], if a[i]=a[j], b[i]<b[j]
    auto cmp=[=](int i, int j){
        if (a[i]>a[j]) return true;
        else if (a[i]<a[j]) return false;
        else {
            return (b[i]>b[j]);
        }
    };
    pq<int,vector<int>,decltype(cmp)>q(cmp);
    l(i,0,n) q.push(i);
    while(!q.empty()){
        int i=q.top();q.pop();
        cout<<i<<" "<<a[i]<<" "<<b[i]<<"\n";
    }

    return 0;
}
/*
5
2 5 5 4 8
1 10 7 7 8
*/