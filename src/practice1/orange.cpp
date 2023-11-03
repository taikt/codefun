// https://leetcode.com/problems/rotting-oranges/description/
class Solution {
public:
    int orangesRotting(vector<vector<int>>& grid) {
        int n = grid.size(), m = grid[0].size();

        queue<pair<int,int>> q;
        // khoi tao: chen tat ca cam hong vao queue
        for (int i = 0; i <n; i++) {
            for (int j = 0; j <m; j++) {
                if (grid[i][j] == 2) q.push({i,j});
            }
        }
        vector<int> dx{0,0,1,-1}, dy{1,-1,0,0};

        int level = 0;
        bool found = 0;
        while (!q.empty()) {
            int size = q.size();
            found = 0;
            for (int i = 0; i<size; i++) {
                auto [x,y] = q.front(); q.pop();
                for (int k=0; k<4; k++) {
                    int u = x + dx[k], v = y + dy[k];
                    if (u <0 || v < 0 || u >= n || v >= m || grid[u][v] == 2 || grid[u][v] == 0) continue;
                    grid[u][v] = 2; // ko can dung mang danh dau visited rieng
                    q.push({u,v});
                    found = 1;
                }
            }
            if (found) level++;
        }

        for (int i = 0; i <n; i++) {
            for (int j = 0; j <m; j++) {
                if (grid[i][j] == 1) return -1;
            }
        }
        return level;
        
    }
};