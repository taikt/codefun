#include <bits/stdc++.h>
using namespace std;

int main() {
    // convert string with split
    // i.e "a b c d" => v[0]=a, v[1]=b...
    #if 1
    freopen("read_string_to_vector.inp","r",stdin);
    freopen("read_string_to_vector.out","w",stdout);
    vector<int> nums;
    vector<string> strs;
    string st;

    // read nums
    getline(cin,st);
    stringstream str(st);
    int t;
    while(str>>t)
        nums.push_back(t);

    for (auto a: nums) {
        cout<<a<<" ";
    }
    cout<<"\n";

    // read strings
    getline(cin,st);
    stringstream str2(st);
    string s;
    while(str2>>s)
        strs.push_back(s);

    for (auto a: strs) {
        cout<<a<<" ";
    }
    #endif // 1

    // convert a string line to a vector
    // i.e "abc" => v="abc"
    #if 0
    //freopen("lc_190.inp","r",stdin);
    //freopen("lc_190.out","w",stdout);
    string s; //="input";
    getline(cin,s);
    vector<char> v(s.begin(),s.end());

    Solution su;
    su.reverseString(v);
    for (auto c:v)
        cout<<c;

    return 0;

    #endif // 0

    return 0;
}
