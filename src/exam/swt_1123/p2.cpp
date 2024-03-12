#include <bits/stdc++.h>
using namespace std;
vector<int> a;
int n,m;
void solve(){
    int ans=-1;
    for (int target=0;target<=15;target++) {
        int l=0,r=0, cnt=0;
        while (l<=r && l<=n-1 && r<=n-1) {
            if (r-l+1<=m) {
                int delt = abs(a[r]-target);
                if (delt <= 1) {
                    cnt++;
                    ans = max(ans, cnt);
                }
            }
            if (r-l+1<m) r++;
            else {
                l++;//new window
                r++;
                if (abs(a[l-1]-target)<=1) cnt--;
            }
        }
    }
    cout<<ans;
}
int main(){
    freopen("input.txt","r",stdin);
    cin>>n>>m;
    a.resize(n,0);
    for (int i=0;i<n;i++) {
        cin>>a[i];
    }
    solve();
    return 0;
}