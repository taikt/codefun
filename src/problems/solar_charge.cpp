#include <bits/stdc++.h>
using namespace std;
int T,N,B,C;
//int p[1000],f[1000],d[1000];
//int cost[1000][1000];

void solve() {
    // tai day i, dung pin
    // cost[i][bat-d[i]] = cost[i-1][bat] if bat >= d[i]
    // tai day i, dung dien
    // cost[i][min(C,bat+p[i])] = cost[i-1][bat] + f[i]*d[i]
    // base case:
    // cost[0][i]=0,B<=i<<C, other=1e9
    /*
    for (int i=0;i<=N;i++){
        for (int j=0;j<=C;j++){
            if (cost[i][j]==1e9) continue;
            if (j>=d[i]) cost[i+1][j-d[i]] = min(cost[i+1][j-d[i]],cost[i][j]);
            cost[i+1][min(C,j+p[i])] = min(cost[i+1][min(C,j+p[i])], cost[i][j] + f[i]*d[i]);
        }
    }
    */
    vector<int> p(N), f(N),d(N);
    int ans=1e9;
    for (int i=0;i<N;i++) cin>>p[i];
    for (int i=0;i<N;i++) cin>>f[i];
    for (int i=0;i<N;i++) cin>>d[i];
    vector<vector<int>> cost(N+1, vector<int>(C+1,1e9));
    cost[0][B] = 0;
    /*
    for (int i=0; i<N; i++) {
        for (int j=0; j<=C; j++) {
            if (cost[i][j] == 1e9) continue;
            int j1 = j - d[i], j2 = min(C, j+p[i]);
            if (j1 >= 0) cost[i+1][j1] = min(cost[i+1][j1], cost[i][j]);
            cost[i+1][j2] = min(cost[i+1][j2], cost[i][j] + f[i]*d[i]);
        }
    }
    */
    for (int i=0;i<=N;i++){
        for (int j=0;j<=C;j++){
            if (cost[i][j]==1e9) continue;
            if (j>=d[i]) cost[i+1][j-d[i]] = min(cost[i+1][j-d[i]],cost[i][j]);
            cost[i+1][min(C,j+p[i])] = min(cost[i+1][min(C,j+p[i])], cost[i][j] + f[i]*d[i]);
        }
    }
    for (int j=B;j<=C;j++) {
            ans = min(ans, cost[N][j]);
    }
    cout<<ans<<"\n";
  
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    cin>>T;
    while (T-- >0){
        cin>>N>>B>>C;
        solve();
    }

    return 0;
}