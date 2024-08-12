// https://codejam.lge.com/contest/problem/1322/4
#include <iostream>
#include <vector>
#include <set>

using namespace std;

const int MOD = 1e9 + 7;

class UnionFind {
public:
    vector<int> par, rnk, sz;

    UnionFind(int N) {
        par.resize(N);
        rnk.resize(N, 0);
        sz.resize(N, 1);
        for (int i = 0; i < N; i++) {
            par[i] = i;
        }
    }

    int find(int a) {
        while (a != par[a]) {
            par[a] = par[par[a]];
            a = par[a];
        }
        return a;
    }

    void merge(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) {
            return;
        }

        if (rnk[a] < rnk[b]) {
            par[a] = b;
            sz[b] += sz[a];
        } else {
            par[b] = a;
            if (rnk[a] == rnk[b]) {
                rnk[a]++;
            }
            sz[a] += sz[b];
        }
    }
};

void find_dpath(int root, const vector<pair<int, int>>& gt, const set<int>& st, vector<int>& dpath) {
    int N = dpath.size();
    UnionFind uf(N);

    // union-find merge
    for (const auto& [p, q] : gt) {
        if (st.find(p) == st.end() && st.find(q) == st.end()) {
            uf.merge(p, q);
        }
    }

    // linking
    vector<vector<int>> adj_dom(N), adj_grp(N);
    for (const auto& [p, q] : gt) {
        bool dom = st.find(p) != st.end();
        bool subdom = st.find(q) != st.end();
        if (dom && !subdom) {
            int g_lnk = uf.find(q);
            adj_grp[p].push_back(g_lnk);
            adj_dom[g_lnk].push_back(p);
        } else if (!dom && subdom) {
            int g_lnk = uf.find(p);
            adj_grp[q].push_back(g_lnk);
            adj_dom[g_lnk].push_back(q);
        }
    }

    // compute dpath
    for (int nd = 0; nd < N; nd++) {
        int base = uf.find(nd);
        if (st.find(nd) != st.end()) {
            dpath[nd] = 1;
            for (int lnk : adj_grp[base]) {
                dpath[nd] += uf.sz[lnk];
            }
        } else {
            if (dpath[base] > 0) {
                dpath[nd] = dpath[base];
                continue;
            }

            dpath[nd] = 0;
            for (int pivot : adj_dom[base]) {
                dpath[nd] += 1;
                for (int lnk : adj_grp[pivot]) {
                    if (uf.find(lnk) != base) {
                        dpath[nd] += uf.sz[lnk];
                    }
                }
            }

            dpath[base] = dpath[nd];
        }
    }
}

void solve(int tc) {
    int na, ma;
    cin >> na >> ma;
    set<int> sa;
    for (int x; cin >> x; ) {
        sa.insert(x - 1);
    }

    vector<pair<int, int>> ga(na - 1);
    for (int i = 0; i < na - 1; i++) {
        int p, q;
        cin >> p >> q;
        ga[i] = {p - 1, q - 1};
    }

    int nb, mb;
    cin >> nb >> mb;
    set<int> sb;
    for (int x; cin >> x; ) {
        sb.insert(x - 1);
    }

    vector<pair<int, int>> gb(nb - 1);
    for (int i = 0; i < nb - 1; i++) {
        int p, q;
        cin >> p >> q;
        gb[i] = {p - 1, q - 1};
    }

    // find dpath
    vector<int> dpath_a(na, 0);
    vector<int> dpath_b(nb, 0);

    find_dpath(0, ga, sa, dpath_a);
    find_dpath(0, gb, sb, dpath_b);

    // find answer
    long long spath_a = 0, spath_b = 0;
    for (int cnt : dpath_a) {
        spath_a = (spath_a + cnt) % MOD;
    }

    for (int cnt : dpath_b) {
        spath_b = (spath_b + cnt) % MOD;
    }

    long long ans = 0;
    for (int i = 0; i < na; i++) {
        ans = (ans + dpath_a[i] * spath_b * (i + 1)) % MOD;
    }

    for (int j = 0; j < nb; j++) {
        ans = (ans + dpath_b[j] * spath_a * (j + 1)) % MOD;
    }

    cout << ans << endl;
}

int main() {
    int tcs;
    cin >> tcs;
    for (int tc = 0; tc < tcs; tc++) {
        solve(tc);
    }
    return 0;
}

