// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/3?embed
// AC
#include <bits/stdc++.h>
using namespace std;
#define db(x) cout<<#x<<"="<<x<<endl;

vector<vector<char>> a;
int n,m;
map<int,pair<int,int>> mk;
vector<int> v;
vector<vector<int>> d(10,vector<int>(10,1e9));

int bfs(int i, int j){
	int sx=mk[i].first, sy=mk[i].second;
	int dex=mk[j].first, dey=mk[j].second;
	queue<vector<int>> q;
	q.push({sx,sy,0});
	vector<vector<bool>> v(n,vector<bool>(m,false));
	v[sx][sy]=true;
	int dx[]={0,0,-1,1};
	int dy[]={1,-1,0,0};
	while(!q.empty()){
		auto cur=q.front();
		q.pop();
		int ci=cur[0], cj=cur[1];
		if (ci==dex && cj==dey) return cur[2];
		for(int k=0;k<4;k++){
			int ni=ci+dx[k], nj=cj+dy[k];
			if (ni<0 || ni>=n || nj<0 || nj>=m) continue;
			if (v[ni][nj] || a[ni][nj]=='*') continue;
			v[ni][nj]=true;
			q.push({ni,nj,cur[2]+1});
		}
	}
	return 1e9;
}

void solve(){
	for(auto &i:v){
		for(auto &j:v){
			if (i==j) continue;
			d[i][j]=bfs(i,j);
		}
	}
	
	v.pop_back();//bo dinh S ra
	sort(v.begin(),v.end());
	int size=v.size(), ans=1e9;
	do{
		int sum=0;
		sum += d[0][v[0]];
		for(int i=0;i<size-1;i++){
			sum+= d[v[i]][v[i+1]];
		}
		sum += d[v[size-1]][0];
		ans=min(sum,ans);
	} while(next_permutation(v.begin(),v.end()));
	
	cout<<ans<<endl;
}

int main(void) {
	cin>>n>>m;
	a.resize(n,vector<char>(m));
	char c;
	for(int i=0;i<n;i++){
		for(int j=0;j<m;j++){
			cin>>c;
			a[i][j]=c;
			if (c=='S') {
				mk[0]={i,j};
			} else if (c>='1'&& c<='9'){
				mk[c-'0']={i,j};
				v.push_back(c-'0');
			}
		}
	}
	v.push_back(0);
	solve();
	return 0;
}