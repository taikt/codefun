// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/2%3Fembed
// AC
#include <bits/stdc++.h>
#define int int64_t
using namespace std;

int N; // 상자의 크기
vector<vector<int>> a;
int total,ans;
void solve() {
	int max_s=0;
	for(int i=0;i<N;i++){
		int temp=0;
		for(int j=0;j<N;j++){
			temp+=a[i][j];
		}
		max_s=max(max_s,temp);
	}
	
	for(int j=0;j<N;j++){
		int temp=0;
		for(int i=0;i<N;i++){
			temp+=a[i][j];
		}
		max_s=max(max_s,temp);
	}
	
	ans=N*max_s - total;
}
int32_t main() {
	cin>>N;
	a.resize(N,vector<int>(N));
	for(int i=0;i<N;i++)
		for(int j=0;j<N;j++){
            cin>>a[i][j];
            total+=a[i][j];
	    }
	solve();

	cout << ans << endl;
	return 0;
}