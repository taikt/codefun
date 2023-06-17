#include "bits/stdc++.h"
using namespace std;

#define FOR(i,a,b) for(int i=a;i<b;i++)

int main() {
    freopen("edges.inp","r",stdin);
    freopen("edges.out","w",stdout);
    int n,m,a,b,c;
    scanf("%d %d",&n,&m);

#if 0
    vector<vector<int>> edges={
        {0,1,3},
        {0,2,5}
        };
#endif // 0

#if 0
    vector<vector<int>> edges; //not init any mem for vector, assign value directly will crash, use push_back is ok.
    FOR(i,0,n) {
        scanf("%d %d %d\n",&a,&b,&c);
        vector<int> v;
        v.push_back(a);
        v.push_back(b);
        v.push_back(c);
        //cout<<a<<" "<<b<<" "<<c<<" "<<"\n";
        edges.push_back(v);
    }
#endif
#if 0
    int default_value =9;
    vector<vector<int>> edges(n,vector<int>(n,default_value)); //init mem for a empty 2-dimensional vector with 0 value,just change value later is ok.
    FOR(i,0,n) {
       scanf("%d %d %d\n",&a,&b,&c);
       edges[i][0] = a;edges[i][1] = b; edges[i][2] = c;
    }
    // crash cases: edges[n][i] = 9; edges[i][n+1],i=[0,n-1]
    // not crash: edges[2][n]??

#endif // 1

#if 0
    vector<pair<int,int>> edges(n);
    FOR(i,0,n) {
       scanf("%d %d %d\n",&a,&b,&c);
       //edges.push_back(make_pair(a,b)); //ok to use vector without initialization
       edges[i]=make_pair(a,b); //need initialization edges(n), otherwise crash
    }
    FOR(i,0,n) {
        cout<<edges[i].first<<edges[i].second<<"\n";
    }
#endif // 1
#if 1
    //vector<vector<pair<int,int>>> edges(n,vector<pair<int,int>>());
    vector<vector<pair<int,int>>> edges(n);
    // {
    //  {{1,2},{2,3}},
    //  {{2,4},{3,7}},
    //  {{2,6},{1,8}}
    // }
    // given input n lines: u,v,w
    // {
    //   {{v1,w1},{v2,w2}}, --> adjList[u]
    //   {{v3,w3},{v4,w2}}, --> adjList[k]

    // }

    FOR(i,0,n) {
       scanf("%d %d %d\n",&a,&b,&c);
       edges[a].push_back(make_pair(b,c)); //need initialize vector by edges(n)(-->init a 1-D vector with size n),otherwise crash
       edges[b].push_back(make_pair(a,c));
    }

    FOR(i,0,n) {
        FOR(j,0,edges[i].size()) {
            cout<<"i="<<i<<","<<edges[i][j].first<<edges[i][j].second<<"\n";
        }
    }

    for(auto a:edges) {
        FOR(j,0,a.size()) {
            cout<<a[j].first<<a[j].second<<"\n";
        }
    }
#endif // 1

    return 0;
}
