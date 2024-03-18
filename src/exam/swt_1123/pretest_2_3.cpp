#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

int dir[4][2] = {
    {0, 1}, // len tren
    {0, -1}, // xuong duoi
    {1, 0}, // sang phai
    {-1, 0}, // sang trai
};

int n, m;
int sx, sy, ex, ey;
vector<vector<int>> mat;

int solve() {
    queue<int> Q;
    Q.push(sx); Q.push(sy);
    mat[sx][sy] = 1;
    
    vector<pair<int, int>> reachable1; reachable1.push_back({sx, sy});
    int nx, ny, x, y;
    while(!Q.empty()) {
        x = Q.front(); Q.pop();
        y = Q.front(); Q.pop();
        if (x == ex && y == ey) return 0;
        
        for (int i = 0; i < 4; ++i) {
            nx = x + dir[i][0];
            ny = y + dir[i][1];
            
            if (nx >= 0 && nx < n && ny >= 0 && ny < m && mat[nx][ny] == 0) {
                Q.push(nx);
                Q.push(ny);
                mat[nx][ny] = 1;
                reachable1.push_back({nx, ny});
            }
        }
    }
    
    // truong hop 2
    Q.push(ex); Q.push(ey);
    mat[ex][ey] = 1;
    vector<pair<int, int>> reachable2; reachable2.push_back({ex, ey});
    while(!Q.empty()) {
        x = Q.front(); Q.pop();
        y = Q.front(); Q.pop();
        
        for (int i = 0; i < 4; ++i) {
            nx = x + dir[i][0];
            ny = y + dir[i][1];
            
            if (nx >= 0 && nx < n && ny >= 0 && ny < m && mat[nx][ny] == 0) {
                Q.push(nx);
                Q.push(ny);
                mat[nx][ny] = 1;
                reachable2.push_back({nx, ny});
            }
        }
    }
    
    int ans = INT_MAX;
    for (auto& p : reachable1) {
        for (auto& p2 : reachable2) {
            ans = std::min(ans, (p.first - p2.first) * (p.first - p2.first) + (p.second - p2.second) * (p.second - p2.second));
        }
    }
    return ans;
}

int main() {
    cin >> n >> m;
    cin >> sx >> sy >> ex >> ey;
    
    mat.resize(n, vector<int>(m, -1));
    int val;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> val;
            if (val == 0) {
                mat[i][j] = 0;
            }
        }
    }
    
    cout << solve() << endl;
    
    return 0;
}
