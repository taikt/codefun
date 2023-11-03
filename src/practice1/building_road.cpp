// https://cses.fi/problemset/task/1666
// https://usaco.guide/silver/graph-traversal
#include <bits/stdc++.h>
using namespace std;

// 4 2
// 1 2
// 3 4
// n city: 4
// 2 roads: 1 2 and 3 4
void solve() {
    int m, n;
    
    cin >> n >> m;
    vector<vector<int>> adj(n);
    vector<bool> visited(n);
    vector<int> trace;
    for (int i=0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        adj[--a].push_back(--b);
        adj[b].push_back(a);
    }

    for (int i = 0; i < n; i++) {
        if (visited[i]) continue;
        visited[i] = true;
        trace.push_back(i);

        // bfs from i
        deque<int> q({i});
        while (!q.empty()) {
            int el = q.front();
            q.pop_front();
            for (auto x : adj[el]) {
                if (visited[x]) continue;
                visited[x] = true;
                q.push_back(x);
            }
        }
    }

    cout << trace.size() - 1 <<"\n";

    for (int i = 0; i < trace.size() - 1; i++) {
        cout<<trace[i]+1 << " "<< trace[i+1] + 1<<"\n";
    }

    return;
}


int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    //freopen("input.txt", "r", stdin);
    solve();

    return 0;
}