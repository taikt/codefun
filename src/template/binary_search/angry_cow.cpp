#include <bits/stdc++.h>
using namespace std;

//const int maxn = 1e8;

int n, k;
#if 1
bool ok(int mid, int ar[]) {
    int start,end, i = 0;
    int cow = 1;
    while (i < n) {
        start = ar[i];
        end  = start + 2*mid;
        while (i < n && ar[i] <= end) {
            i++;
        }
        if (i < n && ar[i] > end) cow++;
    }
    return (cow <= k);
}
#endif

#if 0
bool ok(int mid, int ar[]) {

    int i = 0;
    for (int kas = 0; kas < k && i < n; kas++) {
        int hasta = ar[i];
        hasta += mid * 2;
        while (i < n - 1 && ar[i + 1] <= hasta) i++;
        i++;
    }
    return (i >= n);
}
#endif
int main() {
	freopen("angry.in", "r", stdin);
    //freopen("input.txt", "r", stdin);
	freopen("angry.out", "w", stdout);
	
	cin >> n >> k;
    int ar[n];

	for (int i = 0; i < n; i++) { cin >> ar[i]; }

	sort(ar, ar + n);

	int lo = 0, hi = ar[n-1] - ar[0];
	int sol = 0;

	while (lo < hi) {
		int mid = (lo + hi)/2;
		
		if (ok(mid,ar)) {
			//sol = min(sol, mid);
            sol = mid;
			hi = mid;
		} else {
			lo = mid + 1;
		}
	}

	cout << sol << '\n';
}
