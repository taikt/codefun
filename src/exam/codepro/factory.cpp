#include <bits/stdc++.h>
using namespace std;
#define int long long
main() {
	int n, s;
	cin >> n >> s;
	int cost = 0;
	vector<int> C(n), Y(n);
	for(int i = 0; i < n; ++i) {
		cin >> C[i] >> Y[i];
		//cost += C[i] * Y[i];
	}
	//cout << cost << endl;
	int index = 0;
	cost += C[0] * Y[0];
	for(int i = 1; i < n; ++i) {
		if(C[index] + s * (i - index) > C[i]) {
			index  = i;
		}
		cost += (C[index] + s *(i -index))* Y[i];
	}
	cout << cost << endl;
}

