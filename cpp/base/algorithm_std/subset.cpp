#include <bits/stdc++.h>
using namespace std;
// size n=3
// {}, 1, 2, 5, 1 2, 2 5, 1 5, 1 2 5
// complexity: 2^n  (<=10^8)
// O(2^n): n<=26 
// O(n*2^n): n<=22
// O(n*n*2^n): n<=19
int main(){
    vector<int> v={1,2,5};
    int n=v.size();
    for (int mask=0;mask<(1<<n);mask++){
        vector<int> subset;
        for(int i=0;i<n;i++){
            if (mask&(1<<i)){
                subset.push_back(v[i]);
            }
        }
        //print current subset
        for(auto &x:subset) cout<<x<<" ";
        cout<<endl;
    }
    
    return 0;
}