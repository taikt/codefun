#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
#define int int64_t
using namespace std;
// median of an sorted array (tang dan)
// neu so luong phan tu le
// ans: phan tu chinh giua
// neu so luong phan tu chan
// ans: trung binh cua 2 phan tu o giua
// vd 1 2 6 9
// median: (2+6)/2=4
// vd 1 2 6 9 15
// median: 6

#define MAXNUM 1000 //>phan tu lon nhat trong day

int n;
vector<int> a;
vector<int> f;// tinh so tan suat cua moi a[i]
void solve() {
    sort(a.begin(),a.end());
    f.resize(MAXNUM,0);
    for (int i=0;i<n;i++){
        f[a[i]]++;
    }
    int s=0;
    // O(1)
    for (int i=0;i<MAXNUM;i++){
        s+=f[i];
        if (2*s>n) {
            cout<<i;
            return;
        } else if (2*s==n){
            for (int j=i+1;j<MAXNUM;j++){
                if (f[j]>0){
                    cout<<fixed<<setprecision(2)<<(i+j)/(2*1.0);
                    return;
                } 
            }
        }
    }
}

int32_t main() {
    freopen("input.txt","r",stdin);
    cin >> n;
    a.resize(n);
    for(auto &x: a) cin>>x;

    solve();
    return 0;
}