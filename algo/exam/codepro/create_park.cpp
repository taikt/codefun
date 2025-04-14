// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/5?embed
// AC (bfs)
#include <bits/stdc++.h>
using namespace std;
#define db(x) cout<<#x<<"="<<x<<endl;
int N;
vector<vector<char>> a;
// [0 1 2]: 0 -> R, 1->G, 2->B
// cnt zone -> count grids -> R G B
vector<int> cnt;
vector<int> grid;
vector<vector<bool>> vi;
void bfs(int i, int j){
	int cu;
	char c=a[i][j];
	if (c=='R') cu=0;
	else if (c=='G') cu = 1;
	else cu=2;
	cnt[cu]++;// tang lien thong
	grid[cu]++;
	int dx[]={0,0,1,-1};
	int dy[]={1,-1,0,0};
	queue<pair<int,int>> q;
	q.push({i,j});
	vi[i][j]=true;

	while(!q.empty()){
		auto cur=q.front();
		q.pop();
		int ci=cur.first, cj=cur.second;
		for(int k=0;k<4;k++){
			int ni=ci+dx[k], nj=cj+dy[k];
			if (ni<0 || ni>= N || nj<0 || nj>=N) continue;
			if (a[ni][nj]!= c || vi[ni][nj]) continue;
			grid[cu]++;
			q.push({ni,nj});

			vi[ni][nj]=true;
		}
	}
}
void solve(){
	vi.resize(N,vector<bool>(N,false));

	cnt.resize(3);
	grid.resize(3);
	for(int i=0;i<N;i++){
		for(int j=0;j<N;j++){
			if (!vi[i][j]){
				bfs(i,j);
			}
		}
	}
	vector<int> v={0,1,2};
	sort(v.begin(),v.end(),[&](int i, int j){
		if (cnt[i]>cnt[j]) return true;
		else if (cnt[i]<cnt[j]) return false;
		else {
			if (grid[i]>grid[j]) return true;
			else if (grid[i]<grid[j]) return false;
			else {
				return i<j;// R -> G -> B
			}
		}
	});

	if (v[0]==0) cout<<"R ";
	else if (v[0]==1) cout<<"G ";
	else cout<<"B ";
	cout<<cnt[v[0]]<<endl;
}
int main(){
	cin>>N;
	a.resize(N,vector<char>(N));
	for(int i=0;i<N;i++){
		for (int j=0;j<N;j++)
		cin>>a[i][j];
	}
	solve();
	return 0;
}