#include <bits/stdc++.h>
using namespace std;

int n,t,a[300];


bool f1(int m) {
    return (a[m] > t);
}

// find maximum value <= t
// TTTTTFFFF 
// max(ai) that T
// for ai+1>> F
// 2 3 7 9 10
// so lon nhat nho hon 6 la 3
// F F T T T
// f is 1 if: f(x)>6
// (solution: tim gia tri min ma T truoc, sau do -1)
int bs1(int lo, int hi)
{
   int ans = 0;
   while(lo < hi) {
        int m = lo + (hi - lo) / 2;
        if(f1(m)) {
            ans = m;
            hi = m - 1;
        }
        else
            lo = m + 1;
   }
   return ans - 1;
}

bool f11(int m) {
    return (a[m] <= t);
}
// find maximum value <= t
// TTTTTFFFF 
// max(ai) that T
// for ai+1>> F
// 2 3 7 9 10
// so lon nhat nho hon 6 la 3
// T T F F F
// f is 1 if: f(x)<=6
int bs11(int lo, int hi)
{
   int ans = 0;
   while(lo < hi) {
        int m = lo + (hi - lo) / 2;
        if(!f11(m)) {
            ans = m;
            hi = m - 1;
        }
        else
            lo = m + 1;
   }
   return ans - 1;
}

// find minimum value >= t
// FFFFTTTTTT
// min(ai) that T
// for ai-->> F
// 2 3 7 9 10
// so nho nhat lon 6 la 7
// F F T T T
// f is 1 if: f(x)>6

bool f2(int m) {
    return (a[m] > t);
}

int bs2(int lo, int hi)
{
   int ans = -1;
   while(lo < hi) {
        int m = lo + (hi - lo) / 2;
        if(f2(m)) {
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
    
    cin >>n>>t;
    for (int i=0;i<n;i++){
        cin>>a[i];
    }

   
    cout<<bs2(0,n)<<"\n";
    cout<<bs1(0,n)<<"\n";
    cout<<bs11(0,n)<<"\n";

}

int main() {
    freopen("input.txt","r",stdin);
    solve();

    return 0;
}