#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;
// https://www.geeksforgeeks.org/find-smallest-value-represented-sum-subset-given-array/

const int maxn = 2e5;
int n, a[maxn];

void solve() {
    cin >> n;
    for (int i = 0; i < n; i++) cin >> a[i];

    pr(a,n);
    sort(a, a+n);
    pr(a,n);
    int sum = 1;
    for (int i = 0; i < n; i++) 
    {
        if (a[i] <= sum) {
            cout<<"i="<<i<<", a[i]="<<a[i]<<"\n";
            sum = sum + a[i];
        }
    }

    cout <<sum<<"\n";

    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);

    solve();
    return 0;
}