#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
#define int int64_t
using namespace std;
int n;
vector<vector<vector<int>>> a;
// dp[i][j][t] -> (mincost,maxcost)
// ket thuc tai i, trong tap j(j chua i), voi tap transport method k
// xet moi dinh q trong tap i: q->i (duong di tu q toi i)
// xet moi cach van chuyen trong tap t
// sub_problem
// dp[i][j][t] = dp[q][i^(1<<q)][t^(1<<k)] + a[k][q][i]
// base case
// dp[0][1][0]=0
// min, max below?
// dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n
vector<vector<vector<pair<int,int>>>> dp;
pair<int,int> helper(int i, int j, int t){
    pair<int,int> ans;
    int mincost=1e9, maxcost=0;
    if (dp[i][j][t].first != -1 && dp[i][j][t].second != -1) return dp[i][j][t];
    for (int q=0;q<n;q++){
        if (q!=i && (j&(1<<q))) {
            for (int k=0;k<n;k++){
                if (t&(1<<k) && a[k][q][i]>0){
                    int preset=j-(1<<i), pretran=t-(1<<k);
                    auto pre=helper(q,preset,pretran);
                    mincost=min(mincost, pre.first+a[k][q][i]);
                    maxcost=max(maxcost, pre.second+a[k][q][i]);
                }
            }
        }
    }
    ans.first = mincost;
    ans.second = maxcost;
    cout<<i<<" "<<j<<" "<<mincost<<" "<<maxcost<<endl;
    return dp[i][j][t]=ans;
}
void solve() {
    cin>>n;
    a.resize(n,vector<vector<int>>(n,vector<int>(n,0)));
    dp.resize(n,vector<vector<pair<int,int>>>(n,vector<pair<int,int>>(n,{-1,-1})));
    // k: phuong tien van chuyen
    for (int k=0;k<n;k++){
        // di tu thanh pho i toi j
        for (int i=0;i<n;i++){
            for (int j=0;j<n;j++){
                cin>>a[k][i][j];
            }
        }
    }
    for (int i=0;i<n;i++)
        dp[0][1][i]={0,0};
    // dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n
    int mincost=1e9, maxcost=-1e9;
    cout<<"1"<<endl;
    for (int i=1;i<n;i++){
        for (int k=0;k<n;k++){
            if (a[k][i][0]>0) {
                int pretran=(1<<n)-1 - (1<<k);
                auto ans=helper(i, (1<<n)-1, pretran);
                mincost = min(mincost, ans.first) + a[k][i][0];
                maxcost = min(maxcost, ans.second) + a[k][i][0];
            }
        }
    }
    cout<<mincost<<" "<<maxcost;
}

int32_t main() {
    freopen("input.txt","r",stdin);
    int t;
    cin>>t;
    while(t-->0)
        solve();
    return 0;
}