// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/9%3Fembed
// AC

#include <bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl
using namespace std;
vector<pair<int,int>> pos;
int xA,yA,xB,yB;
int N,ans=1e9;
int min_dist(int x,int y, vector<int>& a){
	int ret=1e9,cost=0, m=a.size();
	if (m==0) return 0;
	do {
		// (x,y) -> a[0] -> .. - a[M-1] -> (x,y)
		cost=0;
		cost += abs(x-pos[a[0]].first) + abs(y-pos[a[0]].second);
		for(int i=0;i<m-1;i++){
			int dist=  abs(pos[a[i]].first -pos[a[i+1]].first) + abs(pos[a[i]].second -pos[a[i+1]].second);
			cost += dist*(1+1+i);
		}
		cost += (abs(x-pos[a[m-1]].first) + abs(y-pos[a[m-1]].second))*(1+m);
		ret=min(ret,cost);
	} while (next_permutation(a.begin(),a.end()));
	return ret;
}
void solve(){
	//tao set N: gia tri 1 => push A
	// gia tri 0 => push B
	for(int mask=0;mask< (1<<N);mask++){
		vector<int> a;
		vector<int> b;
		for(int i=0;i<N;i++){
			if (mask&(1<<i)){
				a.push_back(i);
			} else {
				b.push_back(i);
			}
		}
	
		sort(a.begin(),a.end());
		sort(b.begin(),b.end());
		int cur=min_dist(xA,yA,a) + min_dist(xB,yB,b);
		ans=min(ans,cur);
	}
	cout<<ans<<endl;
}
int main(){
	cin>>N;
	
	cin>>xA>>yA;
	cin>>xB>>yB;
	for(int i=0;i<N;i++){
		int x,y;
		cin>>x>>y;
		pos.push_back({x,y});
	}
	solve();
	return 0;
}