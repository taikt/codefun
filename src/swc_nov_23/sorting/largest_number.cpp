#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

class Solution {
public:
    string largestNumber(vector<int>& nums) {

        if (nums.size() == 0) return "";
        bool all_zero = true;
        vector<string> numarr;
        for (auto x: nums) {
            if (x != 0) all_zero = false;
            numarr.push_back(to_string(x));
        }
        if (all_zero) return "0";

        sort(numarr.begin(), numarr.end(), [](string& a, string& b){
        string str1 = a + b;
        string str2 = b + a;
        int vl = str1.compare(str2);
        if (vl == 0) return true;
        if (vl > 0) return true;
        else return false;
        });
        //cout<<vec;

        string output;
        for (auto x: numarr) {
            output = output + x;
        }
        return output;
    }
};

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    
    string line;
    getline(cin, line);
    stringstream str(line);

    vector<string> vec;
    string num;
    while(str>>num) {
        vec.push_back(num);
    }
    cout<<vec;

    sort(vec.begin(), vec.end(), [](string a, string b){
        string str1 = a + b;
        string str2 = b + a;
        int vl = str1.compare(str2);
        if (vl == 0) return true;
        if (vl > 0) return true;
        else return false;
    });
    cout<<vec;

    string output;
    for (auto x: vec) {
        output = output + x;
    }

    cout<<output<<"\n";

    return 0;
}