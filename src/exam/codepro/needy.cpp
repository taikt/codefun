#include <bits/stdc++.h>
using namespace std;
vector<int> cnt(10);
vector<int> res(10, 0);
vector<int> value = {1, 5, 10, 50, 100, 500, 1000, 3000, 6000, 1200};
vector<int> can_reach;
int cntB = 0;
void solve(int x) {
	if(x <= 0)
		return;
	int index = -1;
	for(int i = 0; i < 10; ++i) {
		if(can_reach[i] == x) {
			for(int j = 0; j <= i; ++j) {
				res[j] = cnt[j];
			}
			return;
		}
		else if(can_reach[i] > x) {
			index = i;
			break;
		}
	}
	if(index == 0) {
		res[index] = x;
		cntB += res[index];
	}
	else 
	{
		res[index] = (x - index?can_reach[index - 1]:0 - 1) / value[index] + 1;
		cntB += res[index];
		solve(x - res[index] * value[index]);
	}
}
main() {
	int n;
	cin >> n;
	int sum = 0;
	for(int i = 0; i < 10; ++i) {
		cin >> cnt[i];
		sum += cnt[i] * value[i];
		can_reach.push_back(sum);
	}
	solve(n);
	cout << cntB << endl;
	for(int i : res)
		cout << i << " ";
}