// https://www.hackerrank.com/contests/swpct-batch-5-pretest-2/challenges/square-color-conversion/problem
// bfs: AC
#include <bits/stdc++.h>

using namespace std;
const int maxn= 1000;
int h, w, a[maxn][maxn];
void solve() {
    cin >> h >> w; 
    char c;
    queue<pair<int,int>> q;
    for (int i = 0; i <h ; i++)
    for (int j = 0; j < w; j++) {
        cin >> c;
        if (c == '.') a[i][j] = 0;
        else if (c == '#') {
            a[i][j] = 1;
            q.push({i,j});
        }
    }
    
    int dx[] = {0,1,0,-1};
    int dy[] = {-1,0,1,0};
    int level = 0;
    while(!q.empty()) {
        int csize = q.size();
        int count = 0;
        level++;
        while (count < csize) {
            pair<int,int> cur = q.front();
            q.pop();
            int cx = cur.first, cy = cur.second;
            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i], ny = cy + dy[i];
                if (nx < 0 || nx >= h || ny <0 || ny >= w || (a[nx][ny] == 1)) continue;
                a[nx][ny] = 1;
                q.push({nx,ny});
            }
            count++;
        }
    }
    if (level != 0)cout<<level-1<<"\n";
    else cout<<"0";
    
    return;
}
int main() {
    solve();
    return 0;
}