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