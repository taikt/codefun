#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
// https://cses.fi/problemset/task/1074

const int MAX_N=2e5;

int n, a[MAX_N];
long long ans=0;
void solve() {
    cin>>n;
    for (int i = 0; i < n; i++) cin>>a[i];

    long long sum=0, med=0;
    sort(a, a+n);
    med = a[n/2];

    for (int i = 0; i < n; i++) {
        ans = ans + abs(a[i] - med);
    } 

    cout<<ans;

    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    solve();

    return 0;
}