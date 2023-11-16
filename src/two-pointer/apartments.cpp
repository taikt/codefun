#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
// https://cses.fi/problemset/task/1084
// https://usaco.guide/problems/cses-1084-apartments/solution

const int MAX_N=2e5;
// 2e5: 2*10^5
int n,m,k,ans=0, a[MAX_N], b[MAX_N];

void solve() {
    cin>>n>>m>>k;
    // a[i]: desized size of appartant by applicants
    // b[i]: true size of apartments
    for (int i = 0; i < n; i++) 
    cin>>a[i];

    for (int i = 0; i < m; i++) 
    cin>>b[i];
    sort(a,a+n);
    // a[i]
    // 45 60 60 80
    sort(b,b+m);
    // b[i]
    // 30 60 75
    int i=0, j=0;
    while (i < n && j < m) {
        if (abs(a[i] - b[j]) <= k) {
            ans++;
            i++;j++;
        } else if (a[i] - b[j] > k) {
            j++;
        } else if (b[j] - a[i] > k) {
            i++;
        }
    }

    cout<<ans;

}
int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);

    solve();

    return 0;
}