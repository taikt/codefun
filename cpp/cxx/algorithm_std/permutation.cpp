#include <bits/stdc++.h>
using namespace std;
// size n=3
// 1 2 5, 1 5 2, 2 1 5, 2 5 1, 5 1 2, 5 2 1
// complexity: n! => (n<=11) (<=10^8)
int main(){
    vector<int> v={1,2,5};
    sort(v.begin(), v.end());
    do {
        for(auto &x:v) cout<<x<<" ";
        cout<<endl;
    } while(next_permutation(v.begin(),v.end()));
    
    return 0;
}