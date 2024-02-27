// https://cses.fi/problemset/task/1668

#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;

int main() {
    //freopen("input.txt","r",stdin);
    // n: number of pupils
    // m: number of relationships
    int n, m;

    cin >> n >> m;
    int x,y;
    vector<vector<int>> a(n);
    for (int i =0 ; i < m; i++) {
        cin >> x >> y;
        a[--x].push_back(--y);
        a[y].push_back(x);
    }
    //cout<<a;

    // bfs for each connected component
    vector<int> v(n,0);
    bool res = true;
    for (int i = 0; i < n; i++) {
        if ((v[i]) != 0) continue;
        // bfs tai v[i]
        queue<int> q;
        q.push(i);
        v[i] = 1; // assign a color for this pupil
        while(!q.empty()) {
            int cu = q.front(); // current
            q.pop();
            //int n; // next
            for (auto ne : a[cu]) {
                // if ne already assigned team, check valid or not
                if (v[ne] != 0) {
                    if (v[cu] == v[ne]) {
                        res = false;                        
                        goto end;
                    } 
                } else {
                        int c;
                        if (v[cu] == 1) c = 2;
                        else c = 1;
                        v[ne] = c;
                        q.push(ne);
                }
            }
        }

    }

end:
    if (!res) cout<<"IMPOSSIBLE\n";
    else {
        for (int i = 0; i < n; i++) {
            cout<<v[i]<<" ";
        }
    }    

    return 0;
}