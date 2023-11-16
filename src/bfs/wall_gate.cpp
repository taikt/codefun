// https://leetcode.com/problems/walls-and-gates/description/
class Solution {
public:
    void wallsAndGates(vector<vector<int>>& r) {
        // day la bai toan template tim shorstest path theo bfs => can bfs theo level
        
        // bfs theo level tu cac gate
        // tim cac gate, add vao queue
        const int INF = 2147483647;
        queue<int> q;
        vector<int> dx={1,-1,0,0}, dy={0,0,1,-1};
        int m = r.size(), n = r[0].size();
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                if (r[i][j] == 0) {
                    q.push(i*n+j);
                }
            }
        }

        // bfs theo level 
        while (!q.empty()) {
            int cnt = q.size();
            // thu khong duyet theo level
            //for (int i= 0; i < cnt; i++) {
                int t = q.front(); q.pop();
                for (int j=0; j<4; j++) {
                    int x = t/n + dx[j], y = t%n + dy[j];
                    if (x<0 || x>=m || y<0 || y >= n || r[x][y] != INF) continue;
                    r[x][y] = r[t/n][t%n] + 1;
                    q.push(x*n+y);
                }
            //}

        }
    }
};