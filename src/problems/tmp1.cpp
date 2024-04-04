#include <bits/stdc++.h>
using namespace std;
int n,m;
// m = p1^a1*p2^a2*p3^a3*p4^a4
// tong so uoc = (a1+1)*(a2+1)..
const int MAXN=1e6+1;
int mark[MAXN];
void era(){
    // sang nguyen to era
    // mark[i]=0: i la so nguyen to
    // mark[i]!0: mark[i] so nguyen to lon nhat ma i chia het
    for (int i=2;i<MAXN;i++) {
        if (mark[i]==0) {
            for (int j=2*i;j<MAXN;j=j+i)
                mark[j]=i;
        }
    }
}

int solve(int m){
    int i=m, ans=1;
    while (i!= 1) {
        int prime = mark[i],cnt=0;
        if (prime == 0) { // i is prime
            ans = ans *2; // mu 1
            break;
        }
        while (i%prime==0){
            cnt++;
            i=i/prime;
        }
        ans = ans * (cnt+1);
    }
    return ans;
}
int main(void) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
	//freopen("input.txt","r",stdin);
    era();
    cin>>n;
    while (n-- >0){
        cin>>m;
        cout<<solve(m)<<"\n";
    }
    
    return 0;
}
