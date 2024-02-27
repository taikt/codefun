// http://www.usaco.org/index.php?page=viewproblem2&cpid=666#
#if 0
#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;

const int maxn = 1e5;
int n,q, a[maxn];

bool f(int m, int e) {
    return (a[m]<=e);
}

bool f2(int m, int s) {
    return (a[m]>=s);
}

int findsm(int e) {
    int lo=0, hi=n-1;
    int ans = 0;
    while(lo<=hi) {
        int m= lo+(hi-lo)/2;
        if (f(m,e)) {
            ans = m;
            lo = m+1;
        } else {
            hi = m-1;
        }
    }
    return ans;
}
int findbg(int s) {
    int lo=0, hi=n-1;
    int ans = n;
    while(lo<=hi) {
        int m= lo+(hi-lo)/2;
        if (f2(m,s)) {
            ans = m;
            hi = m-1;
        } else {
            lo = m+1;
        }
    }
    return ans;
}

void solve() {
    cin >> n>>q;
    for (int i = 0; i < n; i++) cin >> a[i];
    sort(a,a+n);
    int s,e;
    for (int i = 0; i < q; i++){
        cin>>s>>e;
        int h=findsm(e);
        int l=findbg(s);
        cout<<(h-l+1)<<"\n";
    }
    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("haybales.in","r",stdin);
    freopen("haybales.out","w",stdout);



    solve();
    return 0;
}
#endif

#if 1
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

void solve() {
    ll n,q;
    cin>>n>>q;
    vector<ll> a(n);
    for (ll i=0; i<n;i++) cin>>a[i];
    sort(a.begin(), a.end());
    // 2 s 3 5 7 e 10
    // s_index = lower_bound(s)
    // e_index +1 = upper_bound(e)
    // l = e_index - s_index + 1 
    while (q--) {
        ll s,e;
        cin>>s>>e;
        cout<<upper_bound(a.begin(),a.end(),e) - lower_bound(a.begin(), a.end(),s)<<"\n";
    }
}


int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    freopen("haybales.in","r",stdin);
    freopen("haybales.out","w",stdout);

    solve();
    return 0;
}

#endif