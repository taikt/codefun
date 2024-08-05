#include <bits/stdc++.h>
using namespace std;

main() {
	int n;
	cin >> n;
	vector<int> tem(n);
	for(int &x : tem) cin >> x;
	int res1, res2;
	int first = 0, last = n - 1;
	int diff = 2e9 + 5;
	while(first != last) {
		if(diff > abs(tem[first] + tem[last])) {
			res1 = first;
			res2 = last;
			diff = abs(tem[first] + tem[last]);
		}
		if(abs(tem[first]) > abs(tem[last]))
			first++;
		else
			last--;
	}
	cout << res1 << " " << res2 << endl;
}