// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/2
// AC
// O(n*m)=O(1e5)
#include<bits/stdc++.h>
#define int int64_t
using namespace std;
void solve() {
	int n,m;
	cin>>n>>m;
	vector<int> a(n);
	for(int &x:a) cin>>x;
	vector<int> b(m);
	for(int &x:b) cin>>x;
	sort(b.begin(),b.end());
	function<bool(int)> check=[&](int i){
		vector<int> v;
		for(int j=i;j<i+m;j++) v.push_back(a[j]);
		sort(v.begin(),v.end());
		int del=b[0]-v[0];
		for(int k=0;k<m;k++){
			if (del != b[k]-v[k]) return false;
		}
		return true;
	};
	int i=0,cnt=0;
	while (i+m-1<n){
		if (check(i)) cnt++;
		i++;
	}
	cout<<cnt<<endl;
	
}

int32_t main() {
	solve();
	return 0;
}

/*
#include<bits/stdc++.h>
using namespace std;
bool check(vector<int>& x, vector<int>& y) {
	int diff = x[0] - y[0];
	for(int i = 0; i < (int)x.size(); ++i) {
		if(x[i] - y[i] != diff)
			return false;
	}
	return true;
}
int main() {
	int n, m;
	cin >> n >> m;
	vector<int> a(n);
	for(auto& x : a)	cin >> x;
	vector<int> b(m);
	for(auto& x : b) cin >> x;
	sort(b.begin(), b.end());
	int cnt = 0;
	for(int i = 0; i <= n - m; ++i) {
		vector<int> x;
		for(int j = i; j < i + m; ++j) {
			x.push_back(a[j]);
		}
		sort(x.begin(), x.end());
			if(check(x, b)) {
				// cout << i << endl;
				++cnt;
			}
	}
	cout << cnt;
	return 0;
}
*/