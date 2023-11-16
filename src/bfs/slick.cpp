#include <bits/stdc++.h>
using namespace std;

int n, m;
int dx[] = {0,0,-1,1};
int dy[] = {1,-1,0,0};

const int maxn = 300;

int a[maxn][maxn];
bool v[maxn][maxn];
vector<int> res;

int con = 0;

void reset() {
    res.clear();
    for (int i = 0; i < n; i++)
        fill_n(v[i],m,false);
    con = 0;
    //fill_n(a,maxn,0);
}
int bfs(int x, int y) {
    queue<pair<int,int>> q;
    q.push({x,y});
    v[x][y] = true;
    con++;
    int dd = 0;
    while (!q.empty()) {
        dd++;
        int d = q.front().first;
        int e = q.front().second;
        q.pop();
        for (int i = 0; i < 4; i++) {
            int dn = d + dx[i];
            int de = e + dy[i];
            if (dn < 0 || dn >= n || de < 0 || de >= m || v[dn][de] || (a[dn][de] == 0)) continue;
            v[dn][de] = true;
            q.push({dn,de});
        }
    }
    return dd;
}

int main() {
    freopen("input.txt","r",stdin);
        
    while (cin >> n >> m) {
        //cin >> n >> m;
        if (n == 0 && m == 0) return 0;
        int t;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                cin >> a[i][j];
            }
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (a[i][j] == 1 && !v[i][j]) {
                    int cnt = bfs(i,j);
                    res.push_back(cnt);
                }
            }
        }

        // res
        // 22356667
        cout<<con<<"\n";

        int num = 0; 
        res.push_back(1e9);
        sort(res.begin(), res.end());
        int pre = res[0];
        for (int i = 0; i < res.size(); i++) {
            if (pre != res[i]) {
                cout <<pre<<" "<<num<<"\n";
                pre = res[i];
                num = 1;
            } else num++;
        }

        // reset
        reset();
    }

    return 0;
}