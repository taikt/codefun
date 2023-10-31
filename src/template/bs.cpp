#include <bits/stdc++.h>

// find maximum value <= t
// TTTTTFFFF -> FFFFFTTTT
// max(ai) that T
// for ai+1>> F
int bf1() {
    int lo, hi;
    lo--;
    while (lo < hi) {
        int m = lo + (hi - lo + 1)/2;
        if (f(m)) {
            lo = m;
        } else {
            h = m-1;
        }
    }
    return lo;
}

// find minimum value >= t
// FFFFTTTTTT
// min(ai) that T
// for ai-->> F
int bf2() {
    int lo, hi;
    hi++;
    while (lo < hi) {
        int m = lo + (hi - lo)/2;
        if (f(m)) {
            hi = m;
        } else {
            lo = m+1;
        }
    }
    return lo;
}

// always true
{
    int lo, hi;
   int ans;
   while(lo < hi) {
        int m = lo + (hi - lo) / 2;
        if(f(m)) {
            ans = m;
            hi = m - 1;
        }
        else
            lo = m + 1;
   }
   return ans;
}
void solve() 
{
    // input
    // n t
    // a1 a2 .. an
    // find maximum value <= t
    // find minimum value >= t

}

int main() {
    freopen("input.txt","r",stdin);
    solve();
    retur 0;
}