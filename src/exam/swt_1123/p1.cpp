#include <bits/stdc++.h>
//#include "log.hpp"
using namespace std;
int mat[50][50];
int n,m;
#define swap(a,b) {int temp = a; a=b; b=temp; }
void rotate(int sr, int sc, int er, int ec) {
    //if (sr<0 || sc<0 || er<0 || ec<0) cout<<"not ok\n";
    /*
    int a=mat[sr][sc];
    for (int c=sc+1;c<=ec;c++){
        swap(mat[sr][c],a);
    }
    for (int r=sr+1;r<=er;r++){
        swap(mat[r][ec],a);
    }
    for (int c=ec-1;c>=sc;c--){
        swap(mat[er][c],a);
    }
    for (int r=er-1;r>=sr;r--){
        swap(mat[r][sc],a);
    }
    */
    int a=mat[sr][ec];
    for (int c=ec-1;c>=sc;c--){
        swap(mat[sr][c],a);
    }
    
    for (int r=sr+1;r<=er;r++){
        swap(mat[r][sc],a);
    }
    
    for (int c=sc+1;c<=ec;c++){
        swap(mat[er][c],a);
    }
    for (int r=er-1;r>=sr;r--){
        swap(mat[r][ec],a);
    }
}
void solve(void) {
    int size=n>m?n:m;
    for (int i=0;i<size;i++){
        rotate(i,i,n-1-i,m-1-i);
    }

    for (int i=0;i<n;i++){
        for (int j=0;j<m;j++){
            cout<<mat[i][j]<<" ";
        }
        cout<<"\n";
    }
    
}


int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    cin>>n>>m;
    for (int i=0;i<n;i++){
        for (int j=0;j<m;j++){
            cin>>mat[i][j];
        }
    }
    solve();
    return 0;
}