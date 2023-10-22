#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
// https://cses.fi/problemset/task/1073
// https://usaco.guide/problems/cses-1073-towers/solution

const int maxn = 2e5;
int n, x;
vector<int> v;

void solve() {
    cin >> n;
    for (int i = 0; i< n; i++) {
        cin >> x;
        // doesn't need sort v because v is always increasing with below insertion
        //if (v.size() != 0) sort(v.begin(), v.end());
        int upper = upper_bound(v.begin(), v.end(), x) - v.begin();
        if (upper == v.size()) // x is larger than every element in current v
        {
            v.push_back(x); // add new tower
            v[upper] = x; // found a upper potion that is 
                        //smallest element in v but larger than x
                        // add x to top of this tower (having upper one)
        }
    }
    
    cout<<v.size()<<"\n";
    
    return;
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    //freopen("input.txt","r",stdin);

    solve();
    return 0;
}