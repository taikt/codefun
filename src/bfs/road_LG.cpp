#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;



int kc(pair<int,int>&p1, pair<int,int>&p2) {
    return (abs(p1.first - p2.first) + abs(p1.second - p2.second));
}

void solve() {
    int X,x,y, N;
   
    cin >> N >> X;

    cin >> x >> y;
    pair<int,int> des = make_pair(x,y);

    
    vector<pair<int,int> > v;
    for (int i = 0; i < N; i++) {
        cin >> x >> y;
        v.push_back(make_pair(x,y));
    };

    queue<pair<int,int>> q;
    q.push({0,0});

    //cout<<"size of v:"<<v.size()<<"\n";
    
    const int maxn = 10e8;
    vector<bool> visited(maxn, false);

    bool sol = false;
    int cnt = 0;
    //cout<<"N="<<N<<", X=" <<X<<"\n";
    while (!q.empty()) {
        int size = q.size();
        cnt++;
        for (int i = 0; i < size; i++) {
            pair<int,int> cur = q.front();
            //cout<<"cur.x="<<cur.first<<", cur.y="<<cur.second<<"\n";
            visited[cur.first* N + cur.second] = true;
            if (kc(cur, des) <= X) {
                sol = true;
                goto end;
            }
            q.pop();
            pair<int,int> next;
            for (int j = 0 ; j < N; j++) {
                next = v[j];
                //cout<<"next.x="<<next.first<<", next.y="<<next.second<<"\n";
                int kc_cur = kc(cur,next);
                //cout <<"kc to current:"<<kc_cur<<"\n";
                if (kc_cur > X || visited[next.first * N + next.second]) continue;
                visited[next.first * N + next.second] = true;
                q.push(next);
            }
        }
    }

end:
    if (sol) cout<<cnt-1<<"\n";
    else cout<<"-1";

    return;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);

    solve();
    return 0;
}