// https://codeforces.com/contest/1201/problem/C
#include <bits/stdc++.h>
using namespace std;

const int maxn = 2e5+7;
int n,k;
int a[maxn];

bool f(int m) {
    long long s=0;// if int -> wrong answer
    for (int i=(n-1)/2;i<n;i++)
    s=s + max(0, m-a[i]);
    //cout<<"s="<<s<<"k="<<k<<"\n";
    return (s<=k);
}

void solve(){
    cin>>n>>k;
    for (int i=0;i<n;i++)
    cin>>a[i];
    sort(a,a+n);

    int lo=0, hi=2e9;
    int ans=0;
    while (lo <= hi) {
        int m =  lo+(hi-lo)/2;
        if (f(m)) {
            ans=m;
            lo=m+1;
        } else {
            hi=m-1;
        }
    }
    cout<<ans;
    
}

int main() {
    //freopen("input.txt","r",stdin);
    solve();

    return 0;
}