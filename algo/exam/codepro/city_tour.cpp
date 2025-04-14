// https://codejam.lge.com/contest/problem/1322/2
// AC
#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
#define int int64_t
#define INF 3e9 // TLE neu dung chung gia tri INF va MARK!!
// ans>=MARK: ko co duong di
// ans<MARK: co duong di
// ans=INF: khoi tao
#define MARK 2e9
using namespace std;
// dp[i][j][t] -> (mincost,maxcost)
// ket thuc tai i, trong tap j(j chua i), voi tap transport method k
// xet moi dinh q trong tap i: q->i (duong di tu q toi i)
// xet moi cach van chuyen trong tap t
// sub_problem
// dp[i][j][t] = dp[q][j^(1<<i)][t^(1<<k)] + a[k][q][i]
// base case
// dp[0][1][0]=0
// min, max below?
// dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n

void solve() {
    int n;
    cin>>n;
    vector<vector<vector<int>>> a;
    a.resize(n,vector<vector<int>>(n,vector<int>(n)));
    // k: phuong tien van chuyen
    for (int k=0;k<n;k++){
        // di tu thanh pho i toi j
        for (int i=0;i<n;i++){
            for (int j=0;j<n;j++){
                cin>>a[k][i][j];
            }
        }
    }
    
    vector<vector<vector<pair<int,int>>>> dp(n, vector<vector<pair<int,int>>>((1<<n),vector<pair<int,int>>((1<<n),{INF,-INF})));
    dp[0][1][0]={0,0};
    function<pair<int,int>(int,int,int)> helper = [&](int i, int j, int t){
        pair<int,int> ret{MARK, -MARK}; //MARK: neu ans=MARK => khong co duong di (nhung da tinh toan, != INF)
        if (dp[i][j][t].first != INF) {
            //{dp[i][j][t].first,dp[i][j][t].second}; // INF: gia tri khoi tao, chua tinh [i,j,t]
            ret.first=dp[i][j][t].first;
            ret.second=dp[i][j][t].second;
            return ret;
        }
        for (int q=0;q<n;q++){
            if (q!=i && (j&(1<<q))>0) {
                for (int k=0;k<n;k++){
                    if ((t&(1<<k))>0 && a[k][q][i]>0){
                        int preset=j^(1<<i), pretran=t^(1<<k);    
                        auto pre=helper(q,preset,pretran);
                
                        ret.first=min(ret.first, pre.first+a[k][q][i]);
                        ret.second=max(ret.second, pre.second+a[k][q][i]);
                    }
                }
            }
        }
        return dp[i][j][t]=ret;
    };

    int mincost=INF, maxcost=-INF;//khoi tao 

    for (int i=1;i<n;i++){
        for (int k=0;k<n;k++){
            if (a[k][i][0]>0) {
                // dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n
                int pretran=((1<<n)-1) - (1<<k);
                auto ans=helper(i, (1<<n)-1, pretran);
                mincost = min(mincost, ans.first + a[k][i][0]);
                maxcost = max(maxcost, ans.second + a[k][i][0]);
            }
        }
    }
    if (mincost >= MARK) cout<<"0 0"<<endl; // >= MARK nghia la khong co duong di
    else 
    cout<<mincost<<" "<<maxcost<<endl;
}

int32_t main() {
    //freopen("input.txt","r",stdin);
    //ios::sync_with_stdio(false);
    //cin.tie(nullptr);
 
    int t;
    cin>>t;
    while(t-->0)
        solve();
    return 0;
}
/*
// https://codejam.lge.com/contest/problem/1322/2
// AC
#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
#define int int64_t
#define INF 3e9 // TLE neu dung chung gia tri INF va MARK!!
// ans>=MARK: ko co duong di
// ans<MARK: co duong di
// ans=INF: khoi tao
#define MARK 2e9
using namespace std;
int n;
vector<vector<vector<int>>> a;
// dp[i][j][t] -> (mincost,maxcost)
// ket thuc tai i, trong tap j(j chua i), voi tap transport method k
// xet moi dinh q trong tap i: q->i (duong di tu q toi i)
// xet moi cach van chuyen trong tap t
// sub_problem
// dp[i][j][t] = dp[q][j^(1<<i)][t^(1<<k)] + a[k][q][i]
// base case
// dp[0][1][0]=0
// min, max below?
// dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n
vector<vector<vector<int>>> dp_max;
vector<vector<vector<int>>> dp_min;
pair<int,int> helper(int i, int j, int t){
    int mincost=MARK, maxcost=-MARK;//MARK: neu ans=MARK => khong co duong di (nhung da tinh toan, != INF)
 
    if (dp_min[i][j][t] != INF) return {dp_min[i][j][t],dp_max[i][j][t]}; // INF: gia tri khoi tao, chua tinh [i,j,t]
    for (int q=0;q<n;q++){
        if (q!=i && (j&(1<<q))>0) {
            for (int k=0;k<n;k++){
                if ((t&(1<<k))>0 && a[k][q][i]>0){
                    int preset=j^(1<<i), pretran=t^(1<<k);    
                    auto pre=helper(q,preset,pretran);
              
                    mincost=min(mincost, pre.first+a[k][q][i]);
                    maxcost=max(maxcost, pre.second+a[k][q][i]);
                }
            }
        }
    }
    dp_max[i][j][t] = maxcost;
    dp_min[i][j][t] = mincost;
    return {mincost,maxcost};
}
void solve() {
    cin>>n;
    a.clear();// luon nho clear nha may!!!
    dp_min.clear();
    dp_max.clear();
    a.resize(n,vector<vector<int>>(n,vector<int>(n)));
    dp_min.resize(n,vector<vector<int>>((1<<n),vector<int>((1<<n),INF)));
    dp_max.resize(n,vector<vector<int>>((1<<n),vector<int>((1<<n),-INF)));
    // k: phuong tien van chuyen
    for (int k=0;k<n;k++){
        // di tu thanh pho i toi j
        for (int i=0;i<n;i++){
            for (int j=0;j<n;j++){
                cin>>a[k][i][j];
            }
        }
    }
    dp_max[0][1][0]=0;
    dp_min[0][1][0]=0;
    
    int mincost=INF, maxcost=-INF;//khoi tao 

    for (int i=1;i<n;i++){
        for (int k=0;k<n;k++){
            if (a[k][i][0]>0) {
                // dp[i][(1<<n)-1][(1<<n)-1 ^ (1<<k)] + a[k][i][0], i=1,n
                int pretran=((1<<n)-1) - (1<<k);
                auto ans=helper(i, (1<<n)-1, pretran);
                mincost = min(mincost, ans.first + a[k][i][0]);
                maxcost = max(maxcost, ans.second + a[k][i][0]);
            }
        }
    }
    if (mincost >= MARK) cout<<"0 0"<<endl; // >= MARK nghia la khong co duong di
    else 
    cout<<mincost<<" "<<maxcost<<endl;
}

int32_t main() {
    //freopen("input.txt","r",stdin);
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
 
    int t;
    cin>>t;
    while(t-->0)
        solve();
    return 0;
}
*/