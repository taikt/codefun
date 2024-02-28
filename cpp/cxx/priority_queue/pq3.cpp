#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const ll maxi = INT64_MAX;

struct obj {
    int val;
};

auto cmp = [](obj ob1, obj ob2){
    if (ob1.val <= ob2.val) return true;
    else return false;
};

int main() {
    freopen("input.txt","r",stdin);
    int n;
    cin >> n;
    vector<int> a(n);
    for (auto &w: a){
        cin>>w;
    }
    priority_queue<obj, vector<obj>, decltype(cmp)> q;


    for (auto w: a){
        //cout<<w<<" ";
        q.push({w});
    }

    while (!q.empty()){
        auto x = q.top();
        q.pop();
        cout<<x.val<<" ";
    }


    return 0;
}