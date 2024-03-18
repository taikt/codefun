#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>
using namespace std;

int n;
int x, y;
vector<int> arr;

int solve() {
    priority_queue<int, vector<int>, greater<int>> min_heap;
    int diff;
    for (int i = 1; i < n; ++i) {
        diff = arr[i] - arr[i - 1];
        if (diff <= 0) continue;
        
        min_heap.push(diff);
        if ((int)min_heap.size() > x) {
            y -= min_heap.top();
            min_heap.pop();
        }
        if (y < 0) return i - 1;
    }
    
    return n - 1;
}

int main() {
    cin >> n;
    cin >> x >> y;
    arr.resize(n);
    for (int i = 0; i < n; ++i) cin >> arr[i];
    
    cout << solve() << endl;
    return 0;
}
