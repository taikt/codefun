// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/8
#include <bits/stdc++.h>
#define int int64_t
#define INF 1e9
using namespace std;
// dijkstra: AC

int n;
vector<string> a;
int dx[]={0,0,1,-1};
int dy[]={1,-1,0,0};
void solve(){
	priority_queue<vector<int>> q;
	vector<vector<bool>> vi(n,vector<bool>(n,false));
	vector<vector<int>> dist(n,vector<int>(n,INF));
	q.push({0,0,0});
	dist[0][0]=0;
	while(!q.empty()){
		auto cur=q.top();q.pop();
		if (vi[cur[1]][cur[2]]) continue;
		vi[cur[1]][cur[2]] = true;
		for (int k=0;k<4;k++){
			int i=cur[1]+dx[k], j=cur[2]+dy[k];
			if (i<0 || i>=n || j<0 || j>=n) continue;
			if (dist[i][j] > -cur[0] + a[i][j]-'0'){
				dist[i][j] = -cur[0]+a[i][j]-'0';
				q.push({-dist[i][j],i,j});
			}
		}
	}
	cout<<dist[n-1][n-1]<<endl;
}
int32_t main(){
	cin>>n;
	a.resize(n);
	for(auto &x:a) cin>>x;
	solve();
	return 0;
}
