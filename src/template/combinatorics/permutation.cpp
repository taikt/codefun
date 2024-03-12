#include "bits/stdc++.h"
using namespace std;

vector<int> per;
int n;
vector<vector<int>> res;
void solve(vector<int>& a) {
    if (a.size() == n) {
        res.push_back(a);
    }
    for (int i=0;i<n;i++) {
        if (choose[i]) continue;
        choose[i]=true;
        a.push_back(per[i]);
        solve(a);
        choose[i]=false;
        a.pop_back();
    }
}

int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    for (int i=0;i<n;i++){
        int a;
        cin>>a;
        per.push_back(a);
    }

    return 0;
}


#if 0
#include "bits/stdc++.h"
using namespace std;

vector<int> per;
int n;
int main() {
    freopen("input.txt","r",stdin);
    cin>>n;
    for (int i=0;i<n;i++){
        int a;
        cin>>a;
        per.push_back(a);
    }

    do {
        
        for (auto x: per){
            cout<<x<<" ";
        }
        cout<<"\n";
    } while (next_permutation(per.begin(),per.end()));

    return 0;
}
#endif


