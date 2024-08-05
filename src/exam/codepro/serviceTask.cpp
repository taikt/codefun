#include <bits/stdc++.h>
using namespace std;
int main() {
	int n, q;
	cin >> n >> q;
	priority_queue<int> pq;
	for(int i = 0; i < n; ++i) 
		pq.push(0);
	for(int i = 0; i < q; ++i) {
		int x;
		cin >> x;
		int t = pq.top();
		t -= x;
		pq.pop();
		pq.push(t);
	}
	int time = 0;
	while(!pq.empty()) {
		time = max(time, -pq.top());
		pq.pop();
	}
	cout << time;
}