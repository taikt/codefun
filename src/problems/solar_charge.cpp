#include <bits/stdc++.h>
using namespace std;
int T,N,B,C;
int p[1000],f[1000],d[1000];
int cost[1000][1000];

int solve(int i, int bat) {
    // dung pin: bat = bat - d[i], if bat > d[i]
    // dung dien (xac ping): pin = min(pin+p[i],c), cost tang: cost += f[i]*d[i]
    // dk: tai N, bat >= B, cost min
    // cost[i,bat]=min(cost[i-1,bat+d[i]], cost[i-1,bat]+f[i]*d[i])
    // min cost[n,bat], bat >= B
    if (cost[i][bat] != 1e9) return cost[i][bat];
    return (cost[i][bat] = min(min(cost[i-1][bat],cost[i-1][min(bat+d[i],C)]), cost[i-1][bat]+f[i]*d[i])); 
    
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    cin>>T;
    while (T-- >0){
        cin>>N>>B>>C;
        fill_n(p,1000,0);
        fill_n(f,1000,0);
        fill_n(d,1000,0);
        int ans=1e9;
        for (int i=0;i<N;i++) cin>>p[i];
        for (int i=0;i<N;i++) cin>>f[i];
        for (int i=0;i<N;i++) cin>>d[i];
        for (int i=0;i<N;i++){
            for (int bat=0;bat<=C;bat++)
            cost[i][bat]=1e9;
        }
        //for (int i=0;i<B;i++) cost[0][i]=f[0]*d[0];
        for (int i=B;i<=C;i++) cost[0][i]=0;
        for (int j=B;j<=C;j++) ans = min(ans, solve(N-1,j));
        cout<<ans<<"\n";
    }

    return 0;
}