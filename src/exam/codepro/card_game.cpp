// https://codejam.lge.com/contest/problem/1306/2

// dp bitmask
#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
//#define int int64_t
using ll=long long;
using namespace std;

int n,m,t;
vector<int> x,y;
//vector<vector<int>> dp;

bool win(int a, int b){ // check if arbert win at i
    if (a*b>0){
        if (abs(a)>abs(b)) return true;
        else return false;
    } else {
        return true;
    }
}

ll count(int cur_set, int k,vector<vector<ll>>& dp){
    if (dp[cur_set][k] != -1) return dp[cur_set][k];
    //if (k<=0) return 0;
    ll ans=0;
    int size=__builtin_popcountll(cur_set);
    //check at position i
    for (int i=0;i<n;i++){
        if (cur_set & (1<<i)){
            int subset=cur_set-(1<<i);
            if (win(x[size-1],y[i])){
                ans+=count(subset,k-1,dp);
            } else {
                ans+=count(subset,k,dp);
            }
        }
    }
    return (dp[cur_set][k] = ans);
}
void solve() {
    cin>>n>>m;
    x.clear();y.clear();//dp.clear();
    x.resize(n);
    y.resize(n);
    vector<vector<ll>> dp;
    dp.resize((1<<n),vector<ll>(m+1,-1));
    dp[0][0]=1;

    set<int> mask;
    for(int i=0;i<n;i++) {
        cin>>x[i];
        mask.insert(x[i]);
    }
    int k=0;
    for (int i=-n;i<=n;i++){
        if (!mask.count(i) && i!=0) {
            y[k]=i;
            k++;
        }
    }
    //dp[cur_set][k] -> num of ways that arbert has final score k
    // dp[1<<n -1][M]?
    // candidate for last game?
    // size of cur_set=t
    // last of option of Arbert: x[t]
    // select last option of Bob: m: m>==0,m<=t-1 -> y[m]
    // if Arbert wins for last game:
    //  dp[cur_set][k] += dp[cur_set_without_m][k-1] 
    // if Arbert loose last game
    //  dp[cur_set][k] += dp[cur_set_without_m][k]
    // dp[0][0]=1
    cout<<count((1<<n)-1,m,dp)<<endl;
    
}

int32_t main() {
    //freopen("input.txt","r",stdin);
    cin>>t;
    while (t-->0)
        solve();
    return 0;
}