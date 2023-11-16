// https://leetcode.com/problems/number-of-islands/description/
class Solution {
public:
    int numIslands(vector<vector<char>>& grid) {
        int m=grid.size(), n=grid[0].size(); // m hang, n cot
        /*
        a[i,j] => t (t=i*n+j, i=t/n; j=t%n)
        a[i+1,j] => t+n
        a[i-1,j] => t-n
        a[i,j+1] => t+1
        a[i,j-1] => t-1
        */
        vector<vector<bool>> visited(m,vector<bool>(n)); // khoi tao visited all fail, chua tham
        int res=0;
        for (int i=0; i<m; i++) {
            for (int j=0; j<n; j++) {
                if (grid[i][j] == '0' || visited[i][j]) continue; // khong duyet tu dinh nay
                res++; // tang so dao cho moi lan duyet dao lan tu dinh[i][j]
                queue<int> q{{i*n+j}}; // dang lan tu dinh [i][j], moi lan queue q khoi tao moi de lan tu dinh nay, tuy nhien mang visited duoc update
                visited[i][j]=true; // danh dau tham dinh [i][j]
                while (!q.empty()) {
                    int t=q.front(); q.pop();
                    int x=t/n, y=t%n;
                    // chap nhan chon loang sang dinh moi neu dinh do='1' va chua dc tham
                    if (x<m-1 && grid[x+1][y] == '1' && !visited[x+1][y]) {q.push(t+n); visited[x+1][y]=true;} // danh dau dinh [x+1][y]
                    if (x>0 && grid[x-1][y] == '1' && !visited[x-1][y]) {q.push(t-n); visited[x-1][y]=true;}
                    if (y<n-1 && grid[x][y+1] == '1' && !visited[x][y+1]) {q.push(t+1); visited[x][y+1]=true;}
                    if (y>0 && grid[x][y-1] == '1' && !visited[x][y-1]) {q.push(t-1); visited[x][y-1]=true;}
                }
            }
        }
        return res;
        
    }
};