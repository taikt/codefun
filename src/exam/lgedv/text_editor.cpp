// https://www.hackerrank.com/contests/spct2024batch-2-pretest-3/challenges/lg-lite-text-editor/problem
// stack: AC
#include<bits/stdc++.h>
using namespace std;
#define l(i,a,b) for(int i=a;i<b;i++)

int main() {
    stack<string> b;
    string s;
    int n;
    getline(cin,s);
    cin>>n;
    l(i,0,n){
        string cmd;
        cin>>cmd;
        if (cmd=="COPY"){
            int x,y;
            cin>>x>>y;x--;y--;
            b.push(s.substr(x,y-x+1));
        } else if (cmd=="CUT"){
            int x,y;
            cin>>x>>y;x--;y--;
            b.push(s.substr(x,y-x+1));
            s.erase(x,y-x+1);
        } else {//paste
            int x;
            cin>>x;x--;
            if (!b.empty()) s.insert(x,b.top());
            if (b.size()!=1) b.pop();
        }
    }
    cout<<s;
    return 0;
}