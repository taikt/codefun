#include <bits/stdc++.h>
using namespace std;
#define db(x) cout<<#x<<"="<<x<<endl;
int N;
vector<vector<char>> a;
// [0 1 2]: 0 -> R, 1->G, 2->B
// cnt zone -> count grids -> R G B
vector<int> cnt(3);
vector<int> grid(3);
vector<vector<bool>> vi;
void bfs(int i, int j){
	int cu;
	char c=a[i][j];
	if (a[i][j]=='R') cu=0;
	else if (a[i][j]=='G') cu = 1;
	else cu=2;
	cnt[cu]++;// tang lien thong
	grid[cu]++;
	int dx[]={0,0,1,-1};
	int dy[]={1,-1,0,0};
	queue<pair<int,int>> q;
	q.push({i,j});
	vi[i][j]=true;
	db(i);db(j);
	while(!q.empty()){
		auto cur=q.front();
		q.pop();
		int ci=cur.first, cj=cur.second;
		for(int k=0;k<4;k++){
			int ni=ci+dx[k], nj=cj+dy[k];
			if (ni<0 || ni>= N || nj<0 || nj>=N) continue;
			if (a[ni][nj]!= c) continue;
			grid[cu]++;
			q.push({ni,nj});
			vi[ni][nj]=true;
		}
	}
}
void solve(){
	vi.resize(N,vector<bool>(N,false));
	db(N);
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			//db(i);db(j);
			if (!vi[i][j]){
				bfs(i,j);
			}
		}
	}
	db("test");
	for(int i=0;i<3;i++){
		db(i);
		db(cnt[i]);
		db(grid[i]);
	}
}
int main(){
	cin>>N;
	a.resize(N,vector<char>(N));
	for(int i=0;i<N;i++){
		for (int j=0;j<N;j++)
		cin>>a[i][j];
	}
	db("ok");
	solve();
	return 0;
}