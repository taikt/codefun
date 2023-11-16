// https://www.hackerrank.com/contests/swct-pre-testold/challenges/help-a-bartender
#include <bits/stdc++.h>
using namespace std;

void solve() {
    int n,m;
    vector<int> a, b;

    cin>>n>>m;
    //cout<<"n="<<n<<"m="<<m<<"\n";
    int vl;
    for (int i=0;i<n;i++) {
        cin>>vl;
        a.push_back(vl);
    }
    for (int i=0;i<n;i++) {
        cin>>vl;
        b.push_back(vl);
    }
    int x,y;
    vector<vector<int>> v(n);
    for (int i=0;i<m;i++) {
        cin>>x>>y;
        x--;y--;
        v[x].push_back(y);
        v[y].push_back(x);
    }
    vector<bool> vis(n,false);
    //cout<<"start bfs, n="<<n<<"\n";
    bool res = 1;
    for (int i=0; i<n;i++) {
        if (vis[i]) continue;
        vector<int> pos;
        queue<int> q;
        q.push(i);
        vis[i] = true;
        while (!q.empty()) {
            int cur = q.front(); q.pop();
            pos.push_back(cur);
            for (auto nex: v[cur]) {
                if (vis[nex]) continue;
                vis[nex] = true;
                q.push(nex);
            }
        }
        vector<int> temp_a, temp_b;
        for (auto x: pos) {
            temp_a.push_back(a[x]);
            temp_b.push_back(b[x]);
        }
        sort(temp_a.begin(), temp_a.end());
        sort(temp_b.begin(), temp_b.end());
        if (temp_a != temp_b) {res = 0; break;}
        //cout<<"i="<<i<<"\n";
    }
    if (res) cout<<"YES\n";
    else cout<<"NO\n";


}
int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    int t;
    cin>> t;
    while(t--)
        solve();
    return 0;
}