#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

/*
https://leetcode.com/problems/two-sum/

input.txt
3 2 4
6

*/
class Solution {
public:
    vector<int> twoSum(vector<int>& v, int n) {
        unordered_map<int,int> m;
        //vector<int> v{};
        for (int i=0; i<v.size(); i++) {
            m[v[i]] = i; // moi phan tu cua map (key,value): key=thanh phan mang, value=gia tri index
        }

        for (int i=0; i<v.size(); i++) {
            if (m.find(n-v[i]) != m.end() && (i != m[n-v[i]]))
            {
                //cout<<x<<","<<m[x]<<", "<<m[n-x]<<"\n";
                return {i,m[n-v[i]]};
                //break;
            }
        }
        return {};
    }
};

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    
    vector<int> nums;
    int word;
    string st;


    getline(cin,st);
    stringstream str(st);
    while (str>>word)
        nums.push_back(word);

    cout<<"number array is:\n";
    cout<<nums;

    int target;
    cin>>target;

    cout<<"target is: "<<target<<"\n";

    Solution a;


    cout<<"result\n";
    cout<<a.twoSum(nums,target);

    return 0;
}