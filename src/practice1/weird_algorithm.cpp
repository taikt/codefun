// https://cses.fi/problemset/task/1068
#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

int n;

void process(int k) {
    if (k == 1) {
        //cout<<"1\n";
        return;
    }
    if (k%2 == 0) {
        cout<<k/2<<" ";
        process(k/2);
    } else {
        cout<<3*k+1<<" ";
        process(3*k+1);
    }
    
}

void solve() {
    cin >> n;
    cout<<n<<" ";
    process(n);

    return;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    freopen("input.txt", "r", stdin);
    solve();
    return 0;
}
