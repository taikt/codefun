// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/1
// https://leetcode.com/problems/maximum-subarray/solutions/3704593/easy-solution-using-sliding-window/
// prefix sum method: O(n^2): TLE
#include<bits/stdc++.h>
#define int int64_t
using namespace std;

void solve() {
	int n;
	cin>>n;
	vector<int> a(n);
	for(int &x:a) cin>>x;
	vector<int> prefix(n,0);
	prefix[0]=a[0];
	for (int i=1;i<n;i++) prefix[i]=prefix[i-1]+a[i];
	int ans=-1e9;
	for(int i=0;i<n;i++){
		for (int j=i;j<n;j++){
			ans=max(ans, prefix[j]-prefix[i]+a[i]);
		}
	}
	cout<<ans;
	
}

int32_t main() {
	solve();
	return 0;
}