// https://leetcode.com/problems/cheapest-flights-within-k-stops/description/

/*
generally: find shortest path (weight graph) with maximum number of edges of X
=> use bellman-ford: loop x times for relaxation
*/
class Solution {
public:
    int findCheapestPrice(int n, vector<vector<int>>& flights, int src, int dst, int k) {
        // k stop => can tim shortest path voi dk toi da k+1 canh        
        // => lap K+1 step cho bellman-ford

        /*
        lap i=1 toi k+1 
            - lap tat ca cac canh, voi moi canh
                - (u,v):w u->v = w
                - relaxation: gia su co kc dist[u], thu cap nhat g=kc dist[v] qua u
                - if dist[v] > dist[u] + w
                    - dist[v] = dist[u] + w
        */
        if (src == dst) return 0;
        vector<int> pre(n,1e6);
        vector<int> cur(n,1e6);
        pre[src] = 0; // note from input: src index =0 => ko can -1
        for (int i = 1; i <= k+1; i++) {
           cur = pre;
           for (auto ed: flights) {
               // u = ed[0], v = ed[1], w = ed[2] (u->v: w)
               int u = ed[0], v = ed[1], w = ed[2];               
               if (cur[v] > pre[u] + w) cur[v] = pre[u] + w;
               //printf("u=%d, v=%d, w=%d, pre[u]=%d, cur[v]=%d\n",ed[0],ed[1], ed[2], pre[u], cur[v]);
           }
           pre = cur;
        }
        
        return (cur[dst] != 1e6 ) ? cur[dst] : -1;
      }
};

