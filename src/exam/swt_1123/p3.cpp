#include <bits/stdc++.h>
using namespace std;

vector<vector<bool>> v;
int n;
string s;

void solve() {
    int sx=999,sy=999;
    int cnt=0;
    v[999][999]=true;
    for (int i=0;i<n;i++) {
        char c=s[i];
        // 3 north, 0:east, 1: west, 2: south
        if (c=='3') {
            sx--;
        } else if (c=='0') {
            sy++;
        } else if (c=='1') {
            sy--;
        } else if (c=='2') {
            sx++;
        }
        if (v[sx][sy]) {
            cnt++;
        }
        v[sx][sy] = true;
    }
    cout<<cnt;
}

int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    v.resize(2000,vector<bool>(2000,false));
    cin>>s;
    solve();
    return 0;
}