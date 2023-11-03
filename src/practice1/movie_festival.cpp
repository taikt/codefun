//https://cses.fi/problemset/task/1629/
#include <bits/stdc++.h>
using namespace std;
//#include "log.hpp"

int main() {
    ios_base::sync_with_stdio(false);
	cin.tie(NULL);
    //freopen("input.txt","r",stdin);

    int n;
    cin >> n;
    int start, end;
    vector<pair<int, int>> arr;
    for (int i = 0; i < n; i++) {
        cin >> start >> end;
        arr.push_back(make_pair(end, start));
    }
    sort(arr.begin(), arr.end()); // sorting by decreasing end time
    int ans = 0, current_time = 0;
    for (auto el: arr) {

        if (current_time <= el.second) { // el.second is start time
            ans++;
            current_time = el.first; // el.first is end time
        };

    }
   
    cout<<ans;
    return 0;
}
