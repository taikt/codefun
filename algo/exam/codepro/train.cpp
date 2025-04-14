// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/10
// bfs: AC
#include <bits/stdc++.h>
#define int int64_t
using namespace std;
int n,m;
int ret=1e9;
vector<vector<int>> a;
queue<vector<int>> q2; // i,j,distance
int dx[]={0,0,-1,1};
int dy[]={1,-1,0,0};
#define db(x) cout<<#x<<"="<<x<<endl;
// loang phan vung 1 dau tien: gan gia tri 2
// day cac gia tri 1 vao queue 2
void bfs(int i, int j){
	queue<pair<int,int>> q;
	q.push({i,j});
	vector<vector<bool>> vi(n,vector<bool>(m));
	vi[i][j]=true;
	while(!q.empty()){
		auto cur=q.front();
		q.pop();
		q2.push({cur.first,cur.second,0});
		a[cur.first][cur.second]=2;
		for(int k=0;k<4;k++){
			int ni=cur.first+dx[k], nj=cur.second+dy[k];
			if (ni<0 || ni>=n || nj<0 || nj>=m || a[ni][nj]==0 || vi[ni][nj]) continue;
			q.push({ni,nj});
			vi[ni][nj]=true;
		}
	}
}
// bfs tu queue 2
void second_bfs(){
	vector<vector<bool>> vi(n,vector<bool>(m));
	while(!q2.empty()){
		auto cur=q2.front();
		q2.pop();
		if (vi[cur[0]][cur[1]]) continue;
		vi[cur[0]][cur[1]]=true;
		if (a[cur[0]][cur[1]]==1){
			ret=cur[2];
			return;
		}
		for(int k=0;k<4;k++){
			int ni=cur[0]+dx[k], nj=cur[1]+dy[k];
			if (ni<0 || ni>=n || nj<0 || nj>=m || a[ni][nj]==2 || vi[ni][nj]) continue;
			q2.push({ni,nj,cur[2]+1});
		}
	}
}
void solve(){
	bool exit=false;
	for(int i=0;i<n;i++){
		for(int j=0;j<m;j++){
			if (a[i][j]==1) {
				bfs(i,j);
				exit=true;
				break;
			}
		}
		if (exit) break;
	}
	//db(q2.size());
	second_bfs();
	cout<<ret-1<<endl;
}
int32_t main() {
	cin>>n>>m;
	a.resize(n,vector<int>(m));
	for(int i=0;i<n;i++){
		for (int j=0;j<m;j++)
			cin>>a[i][j];
	}
	solve();
	return 0;
}

/*
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
*/