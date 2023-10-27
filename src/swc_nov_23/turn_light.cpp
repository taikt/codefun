#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

void solve() {
    int N, K;
    cin >> N >> K;
    string s;

    cin >> s;

    int lop = 0, i = 0;
    
    while (i < s.size()) {
        if (s[i] == '1') {
            lop++;
            i = i+ K;
        } else {
            i++;
        }
    }
    
    cout<<lop<<"\n";
    
    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);

    //freopen("input.txt","r",stdin);
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    int T;
    cin >> T;
    while (T--) {
        solve();
    }
    return 0;
}