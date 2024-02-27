#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

const int maxn = 2e5;
int n, a[maxn];

int min_diff(int i, int sum1, int sum2) {
    if (i<n) {
        int t1 = min_diff(i+1,sum1+a[i],sum2);
        int t2 = min_diff(i+1,sum1,sum2+a[i]);
        return min(t1,t2);
    } else {
        return abs(sum1-sum2);
    }
}


void solve() {
    cin >> n;
    for (int i = 0; i < n; i++) cin >> a[i];
    cout<<min_diff(0,0,0);

}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    solve();
    return 0;
}

/*
#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

const int maxn = 2e5;
int n, a[maxn];

int min_diff(int i, int sum1, int sum2) {
    if (i<n) {
        int t1 = min_diff(i+1,sum1+a[i],sum2);
        int t2 = min_diff(i+1,sum1,sum2+a[i]);
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