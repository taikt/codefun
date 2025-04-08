// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/8?embed
// AC:
// greedy + monostack
#include <bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl
#define int int64_t
using namespace std;
int N,T;
vector<int> pos,speed;
vector<pair<int,int>> a;

void solve(){
	for(int i=0;i<N;i++){
		a.push_back({pos[i],pos[i]+speed[i]*T});
	}
	sort(a.begin(), a.end());
	vector<int> v(N,-1);
	stack<int> q;
	// tim nho hon gan nhat
	
	for(int i=0;i<N;i++){
		while(!q.empty() && a[q.top()].second >= a[i].second){
			v[q.top()]=i;
			q.pop();
		}
		q.push(i);
	}
	
	int i=0,cnt=0;
	vector<int> ans;
	// vd 10 12 2 1: 1 nhom (10 -> 2 -> 1)
	while(i<N){
		if (v[i]==-1) { 
			ans.push_back(i+1);
			i++;
			cnt++;
		} else {
			while(v[i]>-1){
				i=v[i];
			}
			ans.push_back(i+1);
			cnt++;
			i++;
		}
	}
	cout<<cnt<<endl;
	sort(ans.rbegin(),ans.rend());
	for(auto &i:ans) cout<<i<<" ";
}

int32_t main() {
	cin>>N>>T;
	pos.resize(N);
	speed.resize(N);
	for(int i=0;i<N;i++){
		cin>>pos[i]>>speed[i];
	}
	solve();
	return 0;
}