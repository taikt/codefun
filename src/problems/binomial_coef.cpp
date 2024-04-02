// https://cses.fi/problemset/task/1079

#include <bits/stdc++.h>
using namespace std;
const int p=1e9+7;
int n,a,b;
long long v[1000007][1000007];
/*
int coef(int n,int k, int p){
    if (n==k || k==0) return 1;
    return ((coef(n-1,k-1,p)+coef(n-1,k,p))%p);
}
*/
int coef(int n,int k, int p){
    if (n==k || k==0) return 1;
    if (v[k][n]) return v[k][n];
    return (v[k][n]=(coef(n-1,k-1,p)+coef(n-1,k,p))%p);
}

int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    while (n--){
        cin>>a>>b;
        cout<<coef(a,b,p)<<"\n";
    }
    return 0;
}