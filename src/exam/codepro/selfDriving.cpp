#include <bits/stdc++.h>
using namespace std;
const int r[4] = {0, 0, -1, 1};
const int c[4] = {1, -1, 0, 0};
main() {
    int n, m;
    cin >> n >> m;
    vector<string> map(n);
    for(auto &x : map) cin >> x;
    int visited[n][m];
    memset(visited, 0, sizeof visited);
    queue<tuple<int, int, int> > q;
    q.push({0, 0, 0});
    while(!q.empty()) {
        tuple<int, int, int> l = q.front();
        int x = get<0>(l);
        int y = get<1>(l);
        int t = get<2>(l);
        q.pop();
        if(visited[x][y])
            continue;
        visited[x][y] = t;
        for(int i = 0; i < 4; ++i) {
						int new_x = x+r[i];
            int new_y = y+c[i];
						if(new_x >= 0 && new_y >= 0 && new_x < n && new_y < m)
            if(map[new_x][new_y] == '.' && !visited[new_x][new_y]) {
                q.push(make_tuple(new_x, new_y, t+1));
            }
        }
    }
	if(visited[n-1][m-1] != 0)
    cout << visited[n-1][m-1]  << endl;
	else
		cout << "-1" << endl;
}