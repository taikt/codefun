#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    int n; cin >> n;
    int m; cin >> m;
    vector<int> arr(n);
    for (int i = 0; i < n; ++i) cin >> arr[i];
    
    vector<int> diff;
    for (int i = 1; i < n; ++i) {
        if (arr[i] > arr[i - 1]) diff.push_back(arr[i] - arr[i - 1]); 
    }
    std::sort(diff.begin(), diff.end());
    
    long ans = 0;
    for (int i = 0; i < (int)diff.size() - m; ++i) ans += diff[i];
    cout << ans << endl;
    return 0;
}