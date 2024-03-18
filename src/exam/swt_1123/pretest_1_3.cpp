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
    int ans = 0;
    vector<int> count(32, 0);
    for (int i = 0; i < n; ++i) {
        int b = 0;
        for (int j = 30; j >= 0; --j) {
            if ((arr[i] >> j) & 1) {
                b = j + 1;
                break;
            }
        }
        ans += count[b];
        count[b]++;
    }
    cout << ans << endl;
    return 0;
}