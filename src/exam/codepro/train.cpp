#include<bits/stdc++.h>
using namespace std;
const int nax = 55;
const int r[] = {1, - 1, 0, 0};
const int c[] = {0, 0, - 1, 1};
#define pii pair<int, int>
#define F first
#define S second
bool visited[nax][nax];
int a[nax][nax];
int h, w;
queue<pii> q1;
queue<pair<pii, int>> q2;
int res = 1e9;
void bfs(int x, int y) {
    assert(a[x][y] == 1);
    q1.push({x, y});
    while(!q1.empty()) {
        pii cur = q1.front();
        q1.pop();
        if(visited[cur.F][cur.S])
            continue;
        visited[cur.F][cur.S] = true;
        a[cur.F][cur.S] = 2;
        for(int i = 0; i < 4; ++i) {
            int row = cur.F + r[i];
            int col = cur.S + c[i];
            if(row < h && row >= 0 && col < w && col >= 0 && !visited[row][col] && a[row][col] == 1) {
                q1.push({row, col});
                q2.push(make_pair(make_pair(row, col), 0));
            }
        }
    }
}
void bfs2() {
    while(!q2.empty()) {
        auto cur = q2.front();
        q2.pop();
        if(visited[cur.F.F][cur.F.S])
            continue;
        visited[cur.F.F][cur.F.S] = true;
        for(int i = 0; i < 4; ++i) {
            int row = cur.F.F + r[i];
            int col = cur.F.S + c[i];
            if(row < h && row >= 0 && col < w && col >= 0 && !visited[row][col]) {
                if(a[row][col] == 0) {
                    q2.push(make_pair(make_pair(row, col), cur.S + 1));
                }
                else if(a[row][col] == 1) {
                    res = cur.S;
                    return;
                }
            }
        }
    }
}
int main() {
    memset(visited, 0, sizeof visited);
    cin >> h >> w;
    for(int i = 0; i < h; ++i) {
        for(int j = 0; j < w; ++j) 
            cin >> a[i][j];
    }
    pair<int, int> f = [&](){
        for(int i = 0; i < h; ++i) {
            for(int j = 0; j < w; ++j) {
                if(a[i][j] == 1)
                    return make_pair(i, j);
            }
        }
    }();
    bfs(f.first, f.second);
	// for(int i = 0; i < h; ++i) {
	// 	for(int j = 0; j < w; ++j) {
	// 		cout << a[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }
    q2.push(make_pair(make_pair(f.F, f.S), 0));
    memset(visited, 0, sizeof visited);
    bfs2();
    cout << res << endl;
    return 0;
}