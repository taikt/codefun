#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

// input
// 2 8 5 4 7

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);

    vector<int> nums;
    string line;
    getline(cin, line);

    int value;
    stringstream str(line);
    while(str>>value) {
        nums.push_back(value);
    }
    cout<<nums;
    // increasing
    //sort(nums.begin(), nums.end());

    // decreasing
    sort(nums.begin(), nums.end(), greater<int>());
    cout<<nums;

    return 0;
}