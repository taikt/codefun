// store data structure of a weighted undirected graph
// Tai (taitrananhvn@gmail.com, 20.12.2016)

#include <vector>
#include <cstdio>
#include <utility>
#include <algorithm>
using namespace std;
typedef pair<int,int> ii;

int main() {
	/* 
	4 3 // 4 vertices, 3 edges
	0 2 3 // weight(0,2) = 3
	2 3 1 // weight(2,3) = 1
	0 3 5 // weight(0,3) = 5
	*/
	int N,E;
	int u,v,w;
	

	freopen("input.txt","r", stdin);
	scanf("%d %d",&N,&E);
    
    // list of edges
    // edge(3), <0,2>
    // edge(1), <2,3>
    // edge(5), <0,3>
    //vector<pair<int, pair<int,int>>> EdgeList;

    // list of vertices
    // v(0) -> v(2), w=3
    // v(0) -> v(3), w=5
    // v(2) -> v(3), w=1
    // vector< vector< pair<int,int> > > AdjList;
    vector<pair<int,pair<int,int> > > EdgeList; // vector<pair<int,ii> >
    
    vector< vector<pair<int,int> > > AdjList;
    AdjList.assign(N, vector<pair<int,int> >());
    for (int i = 0; i<E; i++) {
    	scanf("%d %d %d",&u,&v,&w);
    	EdgeList.push_back(make_pair(w, ii(u,v)));

    	AdjList[u].push_back(make_pair(v,w));
    	AdjList[v].push_back(make_pair(u,w));
    }

    printf("EdgeList\n");
    for(vector<pair<int,ii> >::iterator it = EdgeList.begin(); it != EdgeList.end(); it++) {
    	printf("%d %d %d\n", (*it).first, (*it).second.first, (*it).second.second);
    }

    // sorting EdgeList in ascending order
    sort(EdgeList.begin(), EdgeList.end());
    
    printf("EdgeList: After sorting\n");
    for(vector<pair<int,ii> >::iterator it = EdgeList.begin(); it != EdgeList.end(); it++) {
    	printf("%d %d %d\n", (*it).first, (*it).second.first, (*it).second.second);
    }

    printf("VerticeList\n");
    
    for (int i=0; i < N; i++) {
    	printf("vertice %d\n",i);
    	for(vector<pair<int,int> >::iterator it = AdjList[i].begin(); it != AdjList[i].end(); it++) {
    		printf("--to %d, weight=%d\n", (*it).first, (*it).second);
    	}
    }

    return 0;

}
