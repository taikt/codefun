#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

//const int maxN = 2e5+10;
//int a[maxN], X, N;
long long X, N;

bool checkValid(long long H, vector<int>& a) {
    long long total_w = 0;
    //cout<<"H="<<H<<"\n";
    for (int i = 0; i < N; i++) {
        if (a[i] <= H)
            total_w = total_w + H - a[i];
    }
    //cout<<"total_w="<<total_w<<"X="<<X<<"\n";
    return (total_w <= X);
}
void solve() {
  
    cin >> N >> X;
    long long mavl=-1;
    vector<int> a;
    
    //cout<<"N="<<N<<",X="<<X<<"\n";
    for (int i = 0; i < N; i++) {
        int vl;
        cin >> vl;
        a.push_back(vl);
        if (vl > mavl)
            mavl = vl;
    }

    
    //cout<<"maxl="<<mavl<<"\n";
    
    long long  lo=0, hi = 10*X, res = 0;

    while (lo < hi) {
        long long mid = (lo + hi)/2;
        //cout<<"mid="<<mid<<", hi="<<hi<<"\n";
        //if (hi == 0) break;
        if (checkValid(mid,a)) {
            lo = mid+1;
            res = mid;
        } else {
            hi = mid;
        }
    }
    
    cout<<res<<"\n";
    
    
    return;
}

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    //freopen("input.txt","r",stdin);
    int T;
    cin >> T;
    while(T--) {
        solve();
    }
    
    
    return 0;
}