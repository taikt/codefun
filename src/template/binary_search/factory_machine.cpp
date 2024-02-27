// https://cses.fi/problemset/task/1620
#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
ll n,t;

vector<ll> a;
bool f(ll m) {
    ll s = 0;
    for (ll i=0; i<n;i++) {
        if (s>=t) break; // avoid overflow
        s = s+ m/a[i];
    }
    return (s>=t);
}
void solve() {
    cin>>n>>t;
    ll x;
    for (ll i=0; i<n; i++)
    {
        cin>>x;
        a.push_back(x);
    }
    ll lo=0, hi=1e18;
    ll ans = 0;
    while (lo <= hi) {
        ll m=lo+(hi-lo)/2;
        if (f(m)) {
            ans = m;
            hi=m-1;
        } else lo=m+1;
    }
    cout<<ans<<"\n";

}

int main() {
    //freopen("input.txt","r",stdin);
    solve();
    return 0;
}