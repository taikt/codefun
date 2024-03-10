
#if 0
#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const int MAX_N = 20;
const ll MOD = (ll)1e9 + 7;

ll dp[1 << MAX_N][MAX_N];
// come_from[i] contains the cities that can fly to i
vector<int> come_from[MAX_N];

int main() {
	int city_num;
	int flight_num;
	cin >> city_num >> flight_num;
	for (int f = 0; f < flight_num; f++) {
		int start, end;
		cin >> start >> end;
		come_from[--end].push_back(--start);
	}

	dp[1][0] = 1;
	for (int s = 2; s < 1 << city_num; s++) {
		// only consider subsets that have the first city
		if ((s & (1 << 0)) == 0) continue;
		// also only consider subsets with the last city if it's the full subset
		if ((s & (1 << (city_num - 1))) && s != ((1 << city_num) - 1)) continue;

		for (int end = 0; end < city_num; end++) {
			if ((s & (1 << end)) == 0) continue;

			// the subset that doesn't include the current end
			int prev = s - (1 << end);
			for (int j : come_from[end]) {
				if ((s & (1 << j))) {
					dp[s][end] += dp[prev][j];
					dp[s][end] %= MOD;
				}
			}
		}
	}
	cout << dp[(1 << city_num) - 1][city_num - 1] << '\n';
}
#endif 

#if 1
#include "bits/stdc++.h"
using namespace std;
int n,m;
vector<int> adj[20];
using ll= long long;
const int MAX_N = 20;
ll dp[1 << MAX_N][MAX_N];
const ll MOD = (ll)1e9 + 7;
// dp bottom-up
int main() {
    //freopen("input.txt","r",stdin);
    cin>>n>>m;
    for (int i=0;i<m;i++){
        int x,y;
        cin >> x >> y;
        --x,--y;
        adj[y].push_back(x);
    }
    // dp[s][i]: so luong cach di tham het cac dinh trong s va ket thuc tai i
    // s chua i
    // dp[s][i]= sum(dp[s/i][x]) (x->i, x thuoc s)
    // dp[2^n-1][n-1]
    //memset(dp,0,sizeof(dp));
    
    dp[1][0]=1;
    for (int s=2; s< 1<<n; s++){
        if ((s & (1 << 0)) == 0) continue;
		if ((s & (1 << (n - 1))) && s != ((1 << n) - 1)) continue;
        for (int end=0; end<n; end++) {
            //if ((s&(1<<end))==0) continue;
            if ((s&(1<<end))==0) continue;
            int pre = s - (1<<end);
            for (int j: adj[end]){
                // x thuoc s
                if ((s & (1<<j))){
                    dp[s][end] += dp[pre][j];
                    //dp[s][end] = dp[s][end] % (ll)(1e9+7);
                    dp[s][end] %= MOD;
                }
            }
        }  
    }
    
    /*
    dp[1][0] = 1;
	for (int s = 2; s < 1 << n; s++) {
		// only consider subsets that have the first city
		if ((s & (1 << 0)) == 0) continue;
		// also only consider subsets with the last city if it's the full subset
		if ((s & (1 << (n - 1))) && s != ((1 << n) - 1)) continue;

		for (int end = 0; end < n; end++) {
			if ((s & (1 << end)) == 0) continue;

			// the subset that doesn't include the current end
			int prev = s - (1 << end);
			for (int j : adj[end]) {
				if ((s & (1 << j))) {
					dp[s][end] += dp[prev][j];
					dp[s][end] %= MOD;
				}
			}
		}
	}
    */

    cout<<dp[(1<<n)-1][n-1];
  
    return 0;
}
#endif
#if 0
#include "bits/stdc++.h"
using namespace std;
int n,m;
vector<int> adj[22];
using ll= long long;
ll dp[1<<21][21];
// dp top-down
ll solve(int s, int end){
    //if (end == 0) return 0;
    if (dp[s][end]) return dp[s][end];
    ll res=0;
    for (auto &x: adj[end]){
        // x thuoc s
        if (s&(1<<x)){
            res+=solve(s-(1<<end),x);
            res = res % (ll)(1e9+7);
        }
    }
    return (dp[s][end]=res);
}

int main() {
    //freopen("input.txt","r",stdin);
    cin>>n>>m;
    for (int i=0;i<m;i++){
        int x,y;
        cin >> x >> y;
        x--,y--;
        adj[y].push_back(x);
    }
    // dp[s][i]: so luong cach di tham het cac dinh trong s va ket thuc tai i
    // s chua i
    // dp[s][i]= sum(dp[s/i][x]) (x->i, x thuoc s)
    // dp[2^n-1][n-1]
    memset(dp,0,sizeof(dp));
    dp[1][0]=1;
    cout<<solve((1<<n)-1,n-1);
  
    return 0;
}
#endif