#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    int n; cin >> n;
    vector<int> arr(n);
    for (int i = 0; i < n; ++i) cin >> arr[i];
    int k = (1 << n) - 1;
    long ans = 0;
    for (int i = 1; i <= k; ++i) {
        long temp = __builtin_popcount(i);
        for (int j = 0; j < 13; ++j) {
            if ((1 << j) & i) {
                temp ^= arr[j];
            }
        }
        ans += temp;
    }
    cout << ans << endl;
    return 0;
}