#include <bits/stdc++.h>
using namespace std;
int t,k,n;
// c(k,n) = c(k,n-1)+c(k-1,n-1)
// c(k,n)=n!/(k)!.(n-k)!
// 1/k! %M = ((k!)^(M-2))%M // if M prime
// note: (1/x)%M = (x^(M-2))%M
using ll= long long;
const int M=1e9+7;
const int MAXN=1e6;
ll fac[MAXN+1],inv[MAXN+1];
// x^n % M
ll exp1(int x, int n){
    if (n==0) return 1;
    ll u=exp1(x,n/2);
    u = (u*u)%M;
    if (n%2==1) {
        return (u*x)%M;
    } else 
    return u;
}

// n!
void facto(){
    fac[0]=1;
    for (int i=1;i<=MAXN;i++)
        fac[i]=(fac[i-1]*i)%M;
}

// inv
// inv[k]=1/(k)! %M
// inv[k-1]=(inv[k]*k) %M
void cinv(){
    inv[MAXN]=exp1(fac[MAXN],M-2);
    for (int i=MAXN;i>=1;i--)
        inv[i-1]= (inv[i]*i)%M;
}

ll solve(int n,int k) {
    // (x*y)%M = ((x%M)*(y%M))%M
    return (((fac[n]%M)*(inv[k]%M))%M*(inv[n-k]%M))%M;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    facto();
    cinv();
    cin>>t;
    while(t--){
        cin>>n>>k;
        cout<<solve(n,k)<<"\n";
    }
    return 0;
}