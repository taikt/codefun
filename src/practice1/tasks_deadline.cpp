#include "bits/stdc++.h"
using namespace std;
const int maxn = 2e5;

// https://cses.fi/problemset/task/1630
int n;
vector<pair<int,int>> ta;
long long ans;

void solve() {
    cin >> n;
    int x, y;
    for (int i = 0; i < n; i++) {
        cin >> x >> y;
        ta.push_back(make_pair(x,y));
    }
    sort(ta.begin(), ta.end());
    // ta
    // 5 12, 6 10, 8 15
    long long pre_finish=0, f=0, d = 0;
    for (int i =0; i < ta.size(); i++) {
        d = ta[i].second; // ta[i].second is deadline
        f = pre_finish + ta[i].first; // ta[i].first is duration
        ans = ans + (d - f);
        pre_finish = pre_finish + ta[i].first;
    }
    cout<<ans<<"\n";
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    //freopen("input.txt","r",stdin);
    solve();
    return 0;
}