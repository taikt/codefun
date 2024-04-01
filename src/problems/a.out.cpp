#include <bits/stdc++.h>
using namespace std;
int n,m;

int main(void) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
	//freopen("input.txt","r",stdin);
    cin>>n;
    while (n-->0){
        cin>>m;
        int cnt=2;
        for (int i=2;i*i<=m;i++){
            if (m%i==0){
                if (i*i==m)
                    cnt = cnt+1;
                else cnt = cnt+2;
            }
        }
        cout<<cnt<<"\n";
    }
    return 0;
}