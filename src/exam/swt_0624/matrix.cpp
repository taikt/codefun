#include <bits/stdc++.h>
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl
using namespace std;
int n;
void solve() {
    cin>>n;
    vector<vector<int>> a(n,vector<int>(n,0));
    vector<int> res;
    int h=1,w=1;
    int i=n/2,j=n/2, dir1,dir2,t,cnt=2;
    dir1=-1; //h
    dir2=1;//w
    a[i][j]=1;
    while (h*w<=(n-1)*(n-1)){
        t=0;
        while(t<h){
            i+=dir1;
            a[i][j]=cnt++;
            t++;
        }
        h++;
        t=0;
        while(t<w){
            j+=dir2;
            a[i][j]=cnt++;
            t++;
        }
        w++;
        dir1=dir1*-1;
        dir2=dir2*-1;
    }
    for(int i=n-2;i>=0;i--) a[i][0]=cnt++;
    for (int i=0;i<n;i++) {
        for (int j=0;j<n;j++){
            cout<<a[i][j]<<" ";
        }
        cout<<endl;
    }
}

int main() {
    freopen("input.txt","r",stdin);
    solve();
    return 0;
}