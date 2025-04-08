// https://www.hackerrank.com/contests/swpct-practice/challenges/dory-love-food/problem
// Greedy
// AC
#include <bits/stdc++.h>
#define int int64_t
using namespace std;
vector<pair<int,int>> z,ans;
int n,m;
vector<int> a,b;
void solve(){
    sort(z.begin(),z.end());
    int maxcost=m;
    for (int i=0;i<n;i++){
        //select element i at z is freeship day
        maxcost=m-a[z[i].second];
        int sum=0,cnt=0;
        // greedy: chon theo thu tu tang dan theo z (tong a[i]+b[i]) - de duoc nhieu ngay nhat
        for (int j=0;j<n;j++){
            if (j!=i){
                if (sum+z[j].first<=maxcost){
                    sum+=z[j].first;
                    cnt++;
                } else{
                    break;
                }
            }
        }
        sum+=a[z[i].second];
        ans.push_back({cnt+1,-sum});
    }
    sort(ans.begin(),ans.end());
    cout<<ans.back().first<<" "<<-ans.back().second;
}

int32_t main() {
    cin>>n>>m;
    a.resize(n);
    b.resize(n);
    ans.resize(n);
    for(auto &x:a) cin>>x;
    for(auto &x:b) cin>>x;
    for (int i=0;i<n;i++){
        z.push_back({a[i]+b[i],i});
    }
    solve();
    return 0;
}
