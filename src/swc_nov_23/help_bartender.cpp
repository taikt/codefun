#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

const int maxn = 1e3 + 7;
int a[maxn], b[maxn];
int n, m;

bool v[maxn];

bool bfs(int s, int t, vector<int> v1[], vector<int> v2[]) {
    fill_n(v,n,false);
    vector<int> o1;
    vector<int> o2;

    queue<int> q;
    q.push(s);
    v[s] = true;
    while (!q.empty()) {
        int x = q.front();
        o1.push_back(x);
        q.pop();
        for (auto ne : v1[x]) {
            if (!v[ne]) {
                v[ne] = true;
                q.push(ne);
            }
        }
    }
    sort(o1.begin(), o1.end());
    cout<<o1;

    fill_n(v,n,false);
    queue<int> q2;
    //cout<<"t="<<t<<"\n";
    q2.push(t);
    v[t] = true;
    while (!q2.empty()) {
        int x = q2.front();
        o2.push_back(x);
        q2.pop();
        for (auto ne : v2[x]) {
            if (!v[ne]) {
                v[ne] = true;
                q2.push(ne);
            }
        }
    }
    sort(o2.begin(), o2.end());
    //cout<<o2;
    if (o1 == o2) return true;
    else return false;

}
void solve() {
    vector<int> v1[maxn], v2[maxn];
    vector<int> p;

    cin >> n >> m;
    for (int i = 0; i<n; i++) {
        cin >> a[i];
    }
    for (int i = 0; i<n; i++) {
        cin >> b[i];
    }

    for (int i = 0; i<m; i++) {
        int x,y;
        cin >>x>>y;
        v1[a[--x]].push_back(a[--y]);
        v1[a[y]].push_back(a[x]);
        p.push_back(x);

        v2[b[x]].push_back(b[y]);
        v2[b[y]].push_back(b[x]);
    }

    sort(p.begin(), p.end());

    for (auto k: p) {
        if (!bfs(a[k], b[k],v1,v2)) {
            cout<<"NO\n";
            return;
        }
    }
    cout<<"YES\n";

    fill_n(a,n,0);
    fill_n(b,n,0);
   
}

int main() {
    freopen("input.txt","r",stdin);

    int t;
    cin>> t;
    while(t--) {
         solve();
    }
   
    return 0;
}