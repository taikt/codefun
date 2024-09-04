// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/1
// https://leetcode.com/problems/maximum-subarray/description/

// Dynamic Programming: O(n)
// AC
#include<bits/stdc++.h>
#define int int64_t
#define INF 1e9
using namespace std;

void solve() {
	int n;
	cin>>n;
	vector<int> a(n);
	for(int &x:a) cin>>x;
	vector<int> dp(n,INF);
	dp[0]=a[0];
	function<int(int)> helper=[&](int i){
		int ans=-INF;
		if(dp[i]!= INF) return dp[i];
		ans=max(a[i],helper(i-1)+a[i]);
		return dp[i]=ans;
	};
	int sum=-INF;
	for(int i=0;i<n;i++) sum=max(sum,helper(i));
	cout<<sum;
	
}

int32_t main() {
	solve();
	return 0;
}


// sliding window: O(n)
// AC
#include<bits/stdc++.h>
#define int int64_t
using namespace std;

void solve() {
	int n;
	cin>>n;
	vector<int> a(n);
	for(int &x:a) cin>>x;
	int l=0,r=0,ans=-1e9, sum=0;
	// day am -> dich cua sua so left
	while (r<n){
		sum+=a[r];
		ans=max(ans,sum);
		while (sum<0){
			sum-=a[l];
			l++;
		}
		r++;
	}
	
	cout<<ans;
	
}

int32_t main() {
	solve();
	return 0;
}




/*
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
*/