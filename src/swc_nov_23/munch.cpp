#include <bits/stdc++.h>
using namespace std;
const int maxn = 200;
int n,m;
int a[maxn][maxn];

int dx[]={0,0,-1,1};
int dy[]={1,-1,0,0};
int d[maxn][maxn];
bool vi[maxn][maxn];
int bfs() {
    for (int i=0;i<n;i++)
    fill_n(d[i],m,0);

    for (int i=0;i<n;i++)
    fill_n(vi[i],m,false);

    queue<pair<int,int>> q;
    q.push({0,0});
    vi[0][0] = true;
    while(!q.empty()) {
        int cx=q.front().first;
        int cy=q.front().second;
        q.pop();
        for (int i=0; i<4;i++) {
            int u=cx+dx[i];
            int v=cy+dy[i];
            if (u<0 || u>=n || v<0 || v>=m || a[u][v]=='*' || vi[u][v]) continue;
            
            d[u][v] = d[cx][cy] + 1;
            //cout<<"u="<<u<<"v="<<v<<",d="<<d[u][v]<<"\n";
            vi[u][v]=true;
            q.push({u,v});
        }
    }
    return d[n-1][m-1];
}
void solve() {
    cin>>n>>m;
    for (int i=0; i<n;i++)
    for (int j=0;j<m;j++) {
        cin>>a[i][j];
    }
    cout<<bfs();
}
int main() {
    freopen("input.txt","r",stdin);
    solve();
    return 0;
}