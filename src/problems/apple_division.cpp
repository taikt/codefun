// https://cses.fi/problemset/task/1623


#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
using ll = long long;

const int maxn = 20+7;
int n;
ll a[maxn];

void min_diff() {
    ll res=INT64_MAX;
    // 1<<n = 2^n
    for (int mask=0;mask<(1<<n);mask++){
        ll sum1=0,sum2=0;
        for (int i=0;i<n;i++){
            if (mask & (1<<i)) sum1+=a[i];
            else sum2+=a[i];
        }
        res=min(res,abs(sum1-sum2));
    }
    cout<<res;
}


void solve() {
    cin >> n;
    //fill_n(a,n,0);
    for (int i = 0; i < n; i++) cin >> a[i];
    min_diff();
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    solve();
    return 0;
}

/*
#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
using ll = long long;

const int maxn = 20+7;
int n;
ll a[maxn];

ll min_diff(int i, ll sum1, ll sum2) {
    if (i<n) {
        ll t1 = min_diff(i+1,sum1+a[i],sum2);
        ll t2 = min_diff(i+1,sum1,sum2+a[i]);
        return min(t1,t2);
    } else {
        return abs(sum1-sum2);
    }
}


void solve() {
    cin >> n;
    //fill_n(a,n,0);
    for (int i = 0; i < n; i++) cin >> a[i];
    cout<<min_diff(0,0,0);

}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    solve();
    return 0;
}
*/

