#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;
int n,m, a[1000][1000];

void f(int sh, int sc, int dh, int dc) {
    for (int i=sc; i<=dc-1; i++) swap(a[sh][i], a[sh][i+1]);
    for (int i=sh; i<=dh-1; i++) swap(a[i][dh], a[i+1][dh]);
    for (int i=dc; i>=sc+1; i--) swap(a[dh][i], a[dh][i-1]);
    for (int i=dh; i>=sh+2; i--) swap(a[i][sc], a[i-1][sc]);
}
void solve() {
    cin >> n >> m;
    for (int i=0;i<n;i++)
    for (int j=0;j<m;j++) {
        cin>>a[i][j];
    }
    pr(a,n,m);

    int sh= 0, sc=0;
    int dh=n-1, dc=m-1;
    while (sh<dh && sc < dc) {
        f(sh,sc, dh, dc);
        sh++; sc++;
        dh--; dc--;
    }
    
    pr(a,n,m);
    
}

int main() {    
    freopen("input.txt","r",stdin);
    solve();
#if 0   
    // test swap
    int a=1,b=2;
    swap(a,b);
    cout<<a<<b;
#endif
#if 0
    // test swap 1d array
    int a[]={1,2,3};
    for (int i=0;i<2;i++) swap(a[i],a[i+1]);
    //swap(a[0],a[1]);
    //swap(a[1],a[2]);
    for (int i=0;i<3;i++) cout<<a[i];
#endif
#if 0
    // test swap m- array
    int a[3][3]={{1,2,3},{4,5,6},{7,8,9}};
    for (int c=0; c<=1;c++) swap(a[0][c],a[0][c+1]);
    for (int h=0; h<=1;h++) swap(a[h][2],a[h+1][2]);
    for (int c=2; c>=1;c--) swap(a[2][c],a[2][c-1]);
    for (int h=2; h>=2;h--) swap(a[h][0],a[h-1][0]);
    for (int i=0;i<=2;i++)
    {
        for (int j=0;j<=2;j++) cout<<a[i][j];
        cout<<"\n";
    }
#endif    
    return 0;
}