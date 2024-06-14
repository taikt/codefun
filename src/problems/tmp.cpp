#include <bits/stdc++.h>
using namespace std;

string st;
void solve() {
    int n=st.size();
    int row,col;
    row=floor(sqrt(n));
    if (row*row==n) col=row;
    else col=row+1;
    //cout<<"n="<<n<<"\n";
    //cout<<"row="<<row<<"\n";
    string v[100];
    int index=0,jump=0,idm=0,last;
    idm = n/col+(n%col!=0);
    //cout<<"idm="<<idm<<"\n";
    last=n-(idm-1)*col;
    //cout<<"last="<<last<<"\n";
    int i=0;
    while(i<idm) {
        if (i<idm-1){
            for (int j=0;j<col;j++) v[i]+=st[j+jump];
        } 
        else {
            for (int j=0;j<last;j++) v[i]+=st[j+jump];
        }
        i++;
        jump+=col;
    }
    //for (int i=0;i<idm;i++) cout<<v[i]<<"\n";
    #if 1
    string out;
    for (int i=0;i<col;i++){
        for (int j=0;j<idm;j++){
            if (i>=(v[j].size())) continue;
            out+=v[j][i];
        }
        out+=' ';
    }
    cout<<out;
    // v[0]-> 'feed'
    // v[1]-> 'thed'
    // v[2]->'og'
    // v[0][0]+v[1][0]+v[2][0] 
    // v[0][1]+v[1][1]+v[2][1]
    // v[0][2]+v[1][2]
    // v[0][3]+v[1][3]
    #endif
    
    
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    //freopen("input.txt","r",stdin);
    string s;
    cin>>s;
    for(int i=0;i<s.size();i++){
        if (s[i]!=' ') st+=s[i];
    }

    solve();
    return 0;
}