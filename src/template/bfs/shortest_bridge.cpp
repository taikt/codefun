// https://leetcode.com/problems/shortest-bridge/description/
class Solution {
public:
    int shortestBridge(vector<vector<int>>& A) {
        // tim island 1 day vao queue q
        // tim cell 1 dau tien
        queue<int> q;
        vector<int> dx{0,0,1,-1}, dy{1,-1,0,0};
        int startX = -1, startY  = -1;
        int n = A.size(), res = 0;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (A[i][j] == 0) continue;
                startX = i; startY = j;
                //A[i][j] = 2;
                break;
            }
            if (startX != -1) break;
        }

        // Dung DFS tim cac cell cua insland 1 tu (startX, startY)
        DFS(startX, startY, q, A);


        // tu queue q, BFS de loang toi island 2
        while(!q.empty()) {
                //int n=queue.size();
                // duyet tai cac phan tu cua queue o level x
                for (int i=q.size(); i>0; i--) {
                    int t=q.front();q.pop();

                    for (int k = 0; k < 4; k++) {
                        int x=t/n + dx[k], y = t%n + dy[k];
                        if (x < 0 || x >= n || y < 0 || y >= n || A[x][y] == 2) continue;
                        if (A[x][y] == 1) return res;
                        // cell=0, gan gia tri 2, day cell nay vao queue cho duyet level tiep theo
                        A[x][y] = 2;
                        q.push(x*n+y);                
                    }                
                }
                res++; // tang khoang cach bridge len 1, de chuan bi duyet level tiep theo
        }
        return res;
            
    };
    
    void DFS(int x, int y, queue<int>& q, vector<vector<int>>& A) {
        int n = A.size();
        if (x < 0 || x >= n || y < 0 || y >= n || A[x][y] == 0 || A[x][y] == 2) return;
        q.push(x*n + y);
        A[x][y] = 2;
        DFS(x+1,y,q,A);
        DFS(x-1,y,q,A);
        DFS(x,y+1,q,A);
        DFS(x,y-1,q,A);
    }
};

#if 0
class Solution {
public:
    int shortestBridge(vector<vector<int>>& grid) {
        int n=grid.size(), fistCell = -1, res = 0; // grid nxn
        queue<int> q, que; 
        
        // tim cell voi gia tri 1 dau tien cua island 1
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == 0) continue;
                grid[i][j] = 2;
                //q.push(i*n+j);
                fistCell = i*n+j;
                break;
            }
            //if (!q.empty()) break;
            if (fistCell != -1) break;
        }
        
        // tu cell tren BFS de tim tat ca cell cua island 1, gan nhan moi gia tri 2        
        vector<int> dx{1,-1,0,0}, dy{0,0,1,-1};
        q.push(fistCell);
        while(!q.empty()) {
            int t=q.front(); q.pop();
            que.push(t);
            for (int k=0;k<4;k++) {
                int x=t / n + dx[k], y = t % n + dy[k];
                if (x < 0 || x >= n || y < 0 || y >= n || grid[x][y] == 0 || grid[x][y] == 2) continue;
                grid[x][y] = 2;
                q.push(x*n+y);
            }
        }
        
        // thuc hien BFS (level-order traversal) tu tat ca cell cua island 1     
        // duyet tai level x: (tat ca cac phan tu trong queue hien tai)
        // gap cell gia tri 2: bo qua (cung thuoc island 1)
        // gap cell gia tri 0: thanh phan bridge tiem nang, gan gia tri 2, day cell nay vao queue
        // gap cell gia tri 1: tim thay island 2, return ket qua (khoang cach bridge ngan nhat tu island 1 toi island 2)
        // tang khoang cach bridge len 1, de chuan bi duyet level tiep theo
        // duyet tai level x+1
        // ...
        while(!que.empty()) {
            //int n=queue.size();
            // duyet tai cac phan tu cua queue o level x
            for (int i=que.size(); i>0; i--) {
                int t=que.front();que.pop();
                
                for (int k = 0; k < 4; k++) {
                    int x=t/n + dx[k], y = t%n + dy[k];
                    if (x < 0 || x >= n || y < 0 || y >= n || grid[x][y] == 2) continue;
                    if (grid[x][y] == 1) return res;
                    // cell=0, gan gia tri 2, day cell nay vao queue cho duyet level tiep theo
                    grid[x][y] = 2;
                    que.push(x*n+y);                
                }                
            }
            res++; // tang khoang cach bridge len 1, de chuan bi duyet level tiep theo
        }
        return res;
            
    };
};
#endif