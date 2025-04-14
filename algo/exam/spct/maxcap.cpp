/*
5 2 2
0 0 0 0 0
0 1 1 2 0
0 1 4 4 0
0 3 2 1 0
0 0 0 0 0
*/
#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
#define pi pair<int,int>
#define pii pair<int,pi>
#define fi first
#define se second
using namespace std;
int n,x,y;
vector<vector<int>> a;
vector<vector<int>> dp;
int dx[]={1,-1,0,0};
int dy[]={0,0,1,-1};
int get_edge(int &x, int &y){
    if (x==y) return 1;
    return pow(abs(x-y),2);
}
int get_cap(pi s){
    dp.clear();
    dp.resize(n,vector<int>(n,INT_MAX));
    int i=s.fi,j=s.se;
    priority_queue<pii, vector<pii>, greater<pii>> q;
    dp[i][j]=0;
    q.push({0,{i,j}});
    while(!q.empty()) {
        auto cur=q.top();q.pop();
        for (int k=0;k<4;k++){   
            int ni=cur.se.fi+dx[k], nj=cur.se.se+dy[k];
            if (ni>=n-1 || nj>=n-1 || ni<=0 || nj<=0) continue;
            int ed=get_edge(a[cur.se.fi][cur.se.se],a[ni][nj]);
            if (dp[ni][nj] > cur.fi+ed){
                dp[ni][nj]=cur.fi+ed;
                q.push({cur.fi+ed,{ni,nj}});
            }
        }
    }
    assert(dp[x][y]<INT_MAX);
    return dp[x][y];
}
void solve() {
    cin>>n>>x>>y;
    a.resize(n,vector<int>(n));
 
    for(int i=0;i<n;i++)
    for (int j=0;j<n;j++)
    cin>>a[i][j];
    int ans=0;
   
    for (int j=1;j<=n-2;j++){
        ans=max(ans,get_cap({0,j}));
        ans=max(ans,get_cap({n-1,j}));
    }
    for (int i=1;i<=n-2;i++){
        ans=max(ans,get_cap({i,0}));
        ans=max(ans,get_cap({i,n-1}));
    }
    cout<<ans;
}

int main() {
    freopen("input.txt","r",stdin);
    solve();
    return 0;
}