// https://leetcode.com/problems/path-with-minimum-effort/description/
typedef tuple<int,int,int> ti;
class Solution {
public:
    int minimumEffortPath(vector<vector<int>>& h) {
        // 0 <= height <= 1e6
        // binary search + BFS
        // tim xem lieu co ton tai duong di tu src toi dst thoa man dk max diff <= mid
        // neu co duong di
        //    - result = min(result, mid)
        //    - tiep tuc tim kiem trong khong gian hep hon: left=0, right = mid-1
        // neu khong co duong di
        //    - tim kiem tai nua tren: left=mid+1, right = 1e6

        int l=0, r=1e6, res=r;
        int n = h.size(), m = h[0].size();
        //printf("n=%d, m=%d\n",n,m);
        while (l<= r) {
            int mid = (l+r)/2;
            //printf("mid=%d\n",mid);
            if (canFindPath(mid,n,m,h)) {            
                r=mid-1;
                res = min(res, mid);
            } else {
                l=mid+1;
            }
        }
        return res;
    }
    
    bool canFindPath(int mid, int n, int m, vector<vector<int>>& h) {
        // BFS tu source toi dest, neu tim thay dest => true
        queue<pair<int,int>> q;
        q.push({0,0});
        
        vector<vector<int>> visited(n,vector<int>(m,0));
        visited[0][0] = 1;
        vector<int> dx{0,0,1,-1}, dy{1,-1,0,0};
        while (!q.empty()) {           
            auto [x,y] = q.front(); q.pop();            
          
            if (x == n-1 && y == m-1) return true;
            for (int i=0; i<4; i++) {                
                int u=x+dx[i], v=y+dy[i];                
                if (u >= n || v >= m || u<0 || v<0 || visited[u][v]) continue;                
                int diff = abs(h[x][y] - h[u][v]);
                
                if (diff <= mid){
                    q.push({u,v});   
                    visited[u][v] = 1;
                }                
            }           
        }    
        return false;        
    }  
};