// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/11

// dijsktra: AC
// time cua disktra: O(n+m*logm), n so dinh, m so canh
// n=1e2, m=1e4
// => each disktra tai moi dinh: O(1e2+1e4*log(1e4))=1e4
// can disktra tai n dinh: 1e2*O(1e4)=O(1e6)
#include <bits/stdc++.h>
#define int int64_t
#define INF 1e9
using namespace std;
int n,m;
vector<vector<pair<int,int>>> a; //a[x] -> toi dinh b, khoang cach d
int helper(int i){
	priority_queue<pair<int,int>> q;//need min heap: //distance, dinh
	q.push({0,i});
	vector<int> dist(n,INF);
	dist[i]=0;
	vector<int> vi(n);
	
	while(!q.empty()){
		auto cur=q.top();//firt: khoang cach, second: dinh
		q.pop();
		if (vi[cur.second]) continue;
		vi[cur.second]=true;
		vector<pair<int,int>> next=a[cur.second];
		for(auto x:next){// j: <b,distance>
			if (dist[x.first] > -cur.first+x.second){
				dist[x.first] = -cur.first+x.second;
				q.push({-dist[x.first],x.first});
			}
		}
	}
	int ret=dist[0];
	for(int i=0;i<n;i++) ret=max(ret,dist[i]);
	return ret;
}
void solve(){
	int res=1e9;
	for(int i=0;i<n;i++){
		int ret = helper(i);
		res=min(ret,res);
	}
	cout<<res<<endl;
}

int32_t main() {
	cin>>n>>m;
	a.resize(n);
	for(int i=0;i<m;i++){
		int x,y,d;
		cin>>x>>y>>d;
		x--;y--;
		a[x].push_back({y,d});
		a[y].push_back({x,d});
	}
	solve();
	return 0;
}