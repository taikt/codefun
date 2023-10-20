#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;

// input
// 3 30 40
// output
// 40 3 30

template<typename T>
ostream& operator<<(ostream& out, vector<T>& vec) {
    if (vec.size() == 0 ) {
        out<<"{}";
        return out;
    }
    
    out<<"[";
    for (int i = 0; i < vec.size() - 1; i++) {
        out<<vec[i]<<", ";
    }
    out<<vec.back()<<"]\n";
    return out;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);

    vector<string> nums;
    string line;
    getline(cin, line);

    string value;
    stringstream str(line);
    while(str>>value) {
        nums.push_back(value);
    }
    cout<<nums;
    // increasing
    //sort(nums.begin(), nums.end());

    // decreasing
    sort(nums.begin(), nums.end(), [](string a, string b){
        int x = a.compare(b);
        if (x == 0) return true; // a == b
        if (x < 0) return true; // a < b
        else return false; // a > b
    });

    cout<<nums;

    return 0;
}