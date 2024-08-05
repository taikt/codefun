#include<bits/stdc++.h>
using namespace std;

int main() {
    int n;
    cin >> n;
    vector<tuple<int, int, int>> adj;
    vector<vector<int>>  map;
    vector<int> dis(n * n, 1e9);
    for(int i = 0; i < n; ++i) {
        string s;
        cin >> s;
        for(int j = 0; j < n; ++j) {
            int v = n * i + j;
            int d = s[j] - '0';
            if(i > 0)
                adj.push_back({v-n, v, d});
            if(i < (n - 1)) 
                adj.push_back({v+n, v, d});
            if(j > 0)
                adj.push_back({v-1, v, d});
            if(j < (n - 1))
                adj.push_back({v+1, v, d});
        }
        
    }
    dis[0] = 0;
        for(int i = 1; i <= n - 1; ++i) {
            for(auto [x, y, z] : adj) {
                if(dis[y] > dis[x] + z) {
                    dis[y] = dis[x] + z;
                }
            }
        }
    cout << dis[n * n - 1];
}