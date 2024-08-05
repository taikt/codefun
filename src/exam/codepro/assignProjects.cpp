#include <bits/stdc++.h>

using namespace std;
#define pii pair<int, int>
main() {
	int n;
	cin >> n;
	vector<pii> projects(n);
	for(pii &x : projects) {
		cin >> x.first >> x.second;
	}
	sort(projects.begin(), projects.end(), [](pii a, pii b) {
		// if(a.second == b.second) {
		// 	return a.first < b.first;
		// }
		return a.second < b.second;
	});
	int res = 0;
	int end = -1;
	for(int i = 0; i < n; ++i) {
		if(projects[i].first > end) {
			end = projects[i].second;
			res++;
		}
	}
	cout << res << endl;
}