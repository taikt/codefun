#include <bits/stdc++.h>
using namespace std;
#define int long long
main() {
    int n, t;
    cin >> n >> t;
    vector<int> location(n);
    for(int i = 0; i < n; ++i) {
        int start, speed;
        cin >> start >> speed;
        location[i] = start + speed * t;
    }
    int res = 1;
    int max_location = location[n - 1];
    for(int i = n - 2; i >= 0; --i) {
        if(location[i] < max_location) {
            max_location = location[i];
            res++;
        } 
    }
    cout << res << endl;
}