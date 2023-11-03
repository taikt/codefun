/*
//input
2 7 11 15
9
// output
0 1

//input
3 2 4
6
// output
1 2
*/
#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        map<int,int> m;
        for (int i=0; i<nums.size(); i++) {
            m[nums[i]] = i;
        }
        for (int i=0; i<nums.size(); i++) {
            if (m.find(target-nums[i]) != m.end() && (i != m[target - nums[i]])) {
                return {i, m[target-nums[i]]};
            }
        }
        return {};
    }
};

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);

    vector<int> s_arr;
    int number;
    string line;
    

    getline(cin, line);
    stringstream str(line);

    while (str>>number) {
        s_arr.push_back(number);
    }
    int target;
    cin>>target;

    //cout<<s_arr;
    Solution a;
    cout <<a.twoSum(s_arr,target);

    return 0;
}
