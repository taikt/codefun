#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

template<typename T1, typename T2>
ostream& operator<<(ostream& out, map<T1, T2>& m) {
    if (m.size() == 0) {
        out<<"{}";
        return out;
    }
    out<<"[";
    for (auto it=m.begin(); it != m.end(); it++) {
        pair<T1,T2> p = *it;
        out<<p.first<<":"<<p.second;
        if (next(it) != m.end()) out<<", ";
    }
    out<<"]\n";
    return out;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    map<int, string> m;
    m[1] = "tai";
    m[2] = "teo"; 
    cout<<m;

    return 0;
}