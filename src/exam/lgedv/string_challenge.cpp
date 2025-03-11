// https://www.hackerrank.com/contests/spct2024batch-2-pretest-6/challenges/string-challenge-at-lgedv/problem
// bitmask: AC
//#define NDEBUG
#include<bits/stdc++.h>
using namespace std;
#define l(i,a,b) for(int i=a;i<b;i++)
#define db(v) cout<<"line("<<__LINE__<<") -> "<<#v<<"="<<v<<endl;
#define fi first
#define se second
#define pi pair<int,int>
string s;
int n;
vector<pi> v;
void test_case(){
    cin>>s;
    n=s.size();
    //db(n);
    int cnt=0;
    l(i,0,n){
        if (s[i]>='0'&&s[i]<='9'){
            cnt=10*cnt+s[i]-'0';
        } else {
            v.push_back({cnt,s[i]-'a'+1});
            cnt=0;
        }
    }
    int i=0;
    vector<int> mark(32,1);
    int ans=0;
    l(i,0,10) {
        int bit=((n%32)+i)%32;
        ans|=(1<<bit);
        mark[bit]=0;
    }
    
    while(i<v.size()){
        //handle v[i],v[i+1]  
        int tmp=(v[i].fi<<16)|(v[i].se<<11)|(v[i+1].fi<<5)|(v[i+1].se);
        int j=0,d=ans;
        l(i,0,32){
            if (mark[i]){
                //get bit in tmp
                int bit = ((1<<j)&tmp)>0;
                j++;
                d|=(bit<<i);
            }
        }
        cout<<d<<" ";
        assert(j = 22);
        i+=2;
    }
    
}
int main(){
    test_case();
    return 0;
}