// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/3?embed
#include <bits/stdc++.h>
using namespace std;

#define db(x) cout<<"#x"<<"="<<x<<endl;
vector<vector<char>> a;
int n,m;
map<char,pair<int,int>> m;
int main(void) {
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);
	cin>>n>>m;
	a.resize(n,vector<char>(m));
	char c;
	vector<int> v;
	for(int i=0;i<n;i++){
		for(int j=0;j<m;j++){
			cin>>c;
			a[i][j]=c;
			if (c=='S') {
				startX=i;
				startY=j;
			} else if (c>='1'&& c<='9'){
				m[c]={i,j};
				v.push_back(c-'0');
			}
		}
	}
	for (int i=0;i<v.size();i++){
		for(int j=i+1;j<v.size();j++){
			dist[v[i]][v[j]]=bfs(v[i],v[j]);
		}
	}

	return 0;
}