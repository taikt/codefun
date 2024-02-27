#include <bits/stdc++.h>
using namespace std;
//https://cses.fi/problemset/task/1090
// https://usaco.guide/problems/cses-1090-ferris-wheel/solution

const int maxn = 2e5;
int n, p[maxn], x, ans;
bool has_gondo[maxn];

void solve() {
    cin >> n >> x;
    for (int i = 0; i < n; i++) cin>>p[i];
    sort(p, p+n);
    //P: 2 3 7 9
    int i = 0, j = n - 1;
    while (i < j) {
        if (p[i] + p[j] <= x) {
            ans++;
            has_gondo[i] = has_gondo[j] = true;
            i++;
            j--;
        } else {
            has_gondo[j] = false;
            j--;
        }
    }

    for (int i =0; i < n; i++) {
        if (has_gondo[i] == false) ans++;
    }

    cout<<ans<<"\n";

    return;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    //freopen("input.txt","r",stdin);

    solve();
}