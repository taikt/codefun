// 수정전 로봇청소기 1회 사용 시작시간과 종료시간을 이용하여 요금을 계산한 코드
// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/4%3Fembed?embed
// AC
#include <bits/stdc++.h>
using namespace std;
#define db(x) cout<<#x<<"="<<x<<endl
#define int int64_t

string s,e;
int ans;

void solve(){
	// t: total time
	// t<30p: +500
	// 10p' next sau 30p': +300
	// max:30000
	int hs,he,ms,me;
	hs=stoi(s.substr(0,2));
	he=stoi(e.substr(0,2));
	if (he<hs) he+=24;
	ms=stoi(s.substr(3));
	me=stoi(e.substr(3));

	int d=(he-hs)*60 + (me-ms);
	if (d<=0) {
		cout<<0<<endl;
		return;
	} 
	ans =500;
	if (d<=30){
		cout<<ans<<endl;
		return;
	} else {
		d=d-30;	
	}

	ans += d%10==0?(d/10)*300:(d/10)*300+300;
	
	ans=min(ans,(int64_t)30000);
	cout<<ans<<endl;
	
}
int32_t main() {
	cin>>s>>e;
	solve();
}

