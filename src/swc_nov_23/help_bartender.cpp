#include <bits/stdc++.h>
using namespace std;

const int maxn = 1e9 + 7;
int a[maxn], b[maxn];
int n, m;
vector<int> v1[maxn], v2[maxn];
vector<int> p;
bool v[maxn];

bool bfs(int s, int t) {
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

    fill_n(v,n,false);
    queue<int> q2;
    q2.push(t);
    v[t] = true;
    while (!q.empty()) {
        int x = q.front();
        o2.push_back(x);
        q.pop();
        for (auto ne : v2[x]) {
            if (!v[ne]) {
                v[ne] = true;
                q.push(ne);
            }
        }
    }
    sort(o2.begin(), o2.end());

    if (o1 == o2) return true;
    else return false;

}
void solve() {
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
        if (bfs(a[k], b[k])) cout<<"YES\n";
        else cout<<"NO\n";
        
    }
}

int main() {
    freopen("input.txt","r",stdin);

    int t;
    while(t--) {
         solve();
    }
   
    return 0;
}