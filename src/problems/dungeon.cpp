#include <bits/stdc++.h>
using namespace std;
const int maxn=1e7;
long long M=9*1e18;
using ll=long long;
int t,n;
int d[maxn];
int r[maxn];

bool check(string&s, ll m){
    ll h=m;
    for(int i=0;i<n;i++){
        if (h<=d[i]) {
            return false;
        }
        h=((h)-(d[i]));
        if (s[i]=='+') h=h+r[i]%M;
        else if (s[i]=='*') {
            if (h>1e14/r[i]) return true;
            h=h%M*r[i];
        }
    }
    if (h>0) return true;
    else return false;
}
ll solve(string& s) {
    ll ans=0, sum=0;
    int mins=1e9, maxs=0;
  
    ll lo=0,hi=1e14;
    
    while (lo<=hi){
        ll m= lo+(hi-lo)/2;
        if (check(s,m)){
            ans=m;
            hi=m-1;
        } else lo=m+1;
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    cin>>t;
    while (t--){
        cin>>n;
        fill_n(d,n,0);
        fill_n(r,n,0);
        string s;
        for (int i=0;i<n;i++) cin>>d[i];
        cin>>s;
        for (int i=0;i<n;i++) cin>>r[i];
        cout<<solve(s)<<"\n";
    }
    
    return 0;
}