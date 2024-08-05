#include<bits/stdc++.h>
using namespace std;

int main() {
    int n;
    cin >> n;
    vector<pair<int, int>> adj[n*n];
    vector<vector<int>>  map;
    vector<int> dis(n * n, 1e9);
    for(int i = 0; i < n; ++i) {
        string s;
        cin >> s;
        for(int j = 0; j < n; ++j) {
            int v = n * i + j;
            int d = s[j] - '0';
            if(i > 0)
                adj[v - n].push_back({d, v});
            if(i < (n - 1)) 
                adj[v + n].push_back({d, v});
            if(j > 0)
                adj[v - 1].push_back({d, v});
            if(j < (n - 1))
                adj[v + 1].push_back({d, v});
        }
    }
    priority_queue<pair<int, int>> q;
    vector<bool> mark(n * n, 0);
    dis[0] = 0;
    q.push({0,0});
    while(!q.empty()) {
        int u = q.top().second;
		q.pop();
        if(mark[u])
            continue;
        mark[u] = 1;
        for(auto [x, y] : adj[u]) {
            cout << x << " " << y << " " << dis[y] << " " <<  dis[u] + x << '\n';
            if(dis[y] > dis[u] + x) {
                dis[y] = dis[u] + x;
                q.push({-dis[y], y});
            }
        }
    }
    cout << dis[n * n - 1];
}



#include<bits/stdc++.h>

using namespace std;

int main() {
    int n;
    cin >> n;
    vector<vector<int>> map(n, vector<int>(n));
    vector<vector<bool>> visited(n, vector<bool>(n));
    vector<vector<int>> dis(n, vector<int>(n, 1e9));
    for(int i = 0; i < n; ++i) {
        string s;
        cin >> s;
        for(int j = 0; j < n; ++j) {
            map[i][j] = s[j] - '0';
        }
    }
    priority_queue<pair<int, int>> q;
    auto cell_to_id = [&](int row, int col) {
        return row * n + col;
    };
    const int r[] = {0, 0, -1, 1};
    const int c[] = {-1, 1, 0, 0};
    q.push({0, cell_to_id(0, 0)});
    while(!q.empty()) {
        auto p = q.top();
        q.pop();
        int row = p.second / n;
        int col = p.second % n;
        if(visited[row][col])
            continue;
        visited[row][col] = 1;
        dis[row][col] = -p.first;
        if(row == (n - 1) && col == (n - 1))
            break;
        for(int i = 0; i < 4; ++i) {
            int row2 = row + r[i];
            int col2 = col + c[i];
            if(row2 >= 0 && col2 >= 0 && row2 <n && col2 <n && dis[row2][col2] > (dis[row][col] + map[row2][col2]))
                q.push({-(dis[row][col] + map[row2][col2]), cell_to_id(row2, col2)});
        }
    }
    cout << dis[n - 1][n -1] << endl;
    return 0;

}