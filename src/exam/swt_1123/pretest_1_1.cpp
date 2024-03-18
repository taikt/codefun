#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    int n; cin >> n;
    int k; cin >> k;
    vector<int> arr(n);
    for (int i = 0; i < n; ++i) cin >> arr[i];
    
    int maxVal = std::max(arr[0], arr[1]);
    int idx = arr[0] > arr[1] ? 0 : 1;
    int count = 1;
    bool found = false;
    for (int i = 2; i < n; ++i) {
        if (arr[i] > maxVal) {
            count = 1;
            maxVal = arr[i];
        }
        else {
            count++;
        }
        
        if (count == k) {
            cout << maxVal << endl;
            found = true;
            break;
        }
    }
    
    if (!found) cout << maxVal << endl;
    return 0;
}