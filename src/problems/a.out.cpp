#include <bits/stdc++.h>
#define INF_INT 2147483647
#define w(x) cerr << #x << " is " << x << endl;
#define all(v) v.begin(), v.end()
typedef long long ll;
using namespace std;

const int MAX_X = 1e6;
int MAX_DIV[MAX_X + 1];

void precompute() {
    for (int i = 2; i <= MAX_X; i++) {
        if (MAX_DIV[i] != 0) continue;
        for (int j = i; j <= MAX_X; j += i) {
            MAX_DIV[j] = i;
        }
    }
}

int count_divisor(int n) {
    int result = 1;
    while (n != 1) {
        int count = 0, prime = MAX_DIV[n];
		w(n);
		w(prime);
        while (n % prime == 0) {	
            count++;
            n /= prime;
			//w(n);
			//w(count);
        }
        result *= count + 1;
    }
    return result;
}

void solve() {
    precompute();
    int n; cin >> n;

    while (n--) {
        int x; cin >> x;
        cout << (count_divisor(x)) << '\n';
    }
}

int main(void) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
	freopen("input.txt","r",stdin);
    solve();
    return 0;
}