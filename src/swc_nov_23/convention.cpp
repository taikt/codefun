#include <algorithm>
#include <cstdio>
#include <iostream>
#include <vector>

// http://www.usaco.org/index.php?page=viewproblem2&cpid=858
// https://usaco.guide/problems/usaco-858-convention/solution

using std::endl;
using std::vector;

int n, m, c;
vector<int> arrivals;

bool validate(int k) {
	int bus = 0;   // number of buses needed
	int cow = 0;   // current cow
	int lcow = 0;  // earliest cow on this bus
#if 0
	while (cow < n) {
		if (cow == lcow) { bus++; }

		// can't satisfy time constraint by adding this cow
		if (arrivals[cow] - arrivals[lcow] > k) {
			lcow = cow;
			// can't fit this cow in the bus
		} else if (cow - lcow + 1 == c) {
			lcow = ++cow;
			// add this cow to the current bus
		} else {
			cow++;
		}
	}
	return bus <= m;
#endif
    bus = 1;
    while (cow < n) {
        if (cow -lcow + 1 <= c && (arrivals[cow] - arrivals[lcow] <= k)) cow++; // add cow to current bus
        else {
            lcow = cow; // start new bus
            bus++;
        }
    }

    return (bus <= m);
}

int main() {
	freopen("convention.in", "r", stdin);
    //freopen("input.txt", "r", stdin);
	freopen("convention.out", "w", stdout);

	std::cin >> n >> m >> c;
	arrivals.resize(n);

	for (int i = 0; i < n; i++) { std::cin >> arrivals[i]; }

	// sort by arrival time to process cows in order
	sort(arrivals.begin(), arrivals.end());

	int lo = 0, ans = 0;
    //int hi = arrivals[n - 1] - arrivals[0] // original
	int hi = arrivals[n - 1] - arrivals[0] + 100; // ok: any value is equal or larger than arrivals[n - 1] - arrivals[0] 

	while (lo < hi) {
		int mid = (lo + hi)/2;

		if (validate(mid)) {
			hi = mid;
            ans = mid;
		} else {
			lo = mid + 1;
		}
	}
	std::cout << ans << endl;
}