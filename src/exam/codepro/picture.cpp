#include <bits/stdc++.h>
using namespace std;
#define pii pair<int, int>
vector<bool> valid(10, 1);
vector<bool> has(10);
vector<int> topX(10, 11);
vector<int> topY(10, 11);
vector<int> downX(10);
vector<int> downY(10);
int main() {
	int n;
	cin >> n;
	vector<string> s(n);
	for(string &i : s) {
			cin >> i;
	}
	for(int i = 0; i < n; ++i) {
		for(int j = 0; j < n; ++j) {
			char c = s[i][j];
			topX[c - '0'] = min(topX[c - '0'], j);
			topY[c - '0'] = min(topY[c - '0'], i);
			downX[c - '0'] = max(downX[c - '0'], j);
			downY[c - '0'] = max(downY[c - '0'], i);
			has[c - '0'] = 1;
		}
	}
	for(int i = 1; i < 10; ++i) {
		if(has[i]) {
			//cout << i << " " << topY[i] << " " << topX[i] << " " << downY[i] << " " << downX[i] << endl;
			for(int j = topY[i]; j <= downY[i]; ++j) {
				for(int k = topX[i]; k <= downX[i]; ++k) {
					int d = s[j][k] - '0';
                    assert(d != 0);
					if(d != i)
						valid[d] = 0;
				}
			}
		}
	}
	int ans = 0;
	for(int i = 1; i < 10; ++i) {
		if(valid[i] && has[i])
			++ans;
	}
	// for(int i : has) cout << i << " ";
	// cout << endl;
	// for(int i : valid) cout << i << " ";
	// cout << endl;
	cout << ans;
	//for(string &i : s) cout << i;
	return 0;
}