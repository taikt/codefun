// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/14
// AC
#include <bits/stdc++.h>
#define db(x) cout<<#x<<"="<<x<<endl;
#define int int64_t
using namespace std;
int dx[]={0,0,1,-1};
int dy[]={1,-1,0,0};
int N;
string s;
// 0: dong x 0,y +1
// 1: tay x 0 y -1
// 2: nam x 1, y 0
// 3: bac x -1 y 0
set<pair<int,int>> m;
set<vector<int>> v;
void solve(){
	char c;
	int x=0,y=0,nx,ny,ans=0;

	for(int i=0;i<N;i++){
		c=s[i];
		nx=x+dx[c-'0'];
		ny=y+dy[c-'0'];
		// da tham diem nhung chua tham canh
		if (m.count({nx,ny}) && !v.count({x,y,nx,ny})){
			ans++;
		}
		// danh dau diem
		m.insert({x,y});
		// danh dau canh da tham (2 huong)
		v.insert({x,y,nx,ny});
		v.insert({nx,ny,x,y});
		x=nx;
		y=ny;
	}
	
	cout<<ans<<endl;
}
int32_t main(){
	cin>>N;
	cin>>s;
	solve();
	return 0;
}