#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;

const int maxn = 2e5;
int n, cnt[26];
string s;
vector<string> res;

void solve(string& st) {
    if (st.length()==n){
        res.push_back(st);
        return;
    }
    for (int i=0;i<26;i++){
        if (cnt[i]>0){
            cnt[i]--;
            st=st+(char)('a'+i);
            solve(st);
            st.pop_back();
            cnt[i]++;
        }
    }

    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);

    cin>>s;
    for (auto x: s){
        cnt[x-'a']++;
    }
    n=s.length();
    string st;
    solve(st);
    cout<<res.size()<<"\n";
    for (auto x: res){
        cout<<x<<"\n";
    }
    return 0;
}