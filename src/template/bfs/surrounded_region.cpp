// https://leetcode.com/problems/surrounded-regions/description/
/*
BFS method: lan vung 'O' tu bien bang BFS, tat ca vung lan den duoc danh dau '$'
	Nhung vung 'o" khong lan toi duoc la surrounded region.
	Gan lai cac vung gia tri 'O' thanh 'X', vung gia tri '$' thanh 'O'.
*/
class Solution {
public:
#if 1    
    void solve(vector<vector<char>>& board) {
        if (board.empty() || board[0].empty()) return;
        //lan tu bien
        // so hang: 
        int m= board.size();
        // so cot;
        int n=board[0].size();
        
        for (int i=0; i<m; i++) {
            for (int j=0; j<n; j++) {
                if ((i==0 || i==m-1 || j==0 || j==n-1) && board[i][j] == 'O') 
                    BFS(board,i,j);               
            }
        }
        /*
        // error
        for (int i=0; i<m; i++) {
            for (int j=0; j<n; j++) {
                if (board[i][j] == '$') board[i][j] == 'O';
                else if (board[i][j] == 'O') board[i][j] == 'X';
            }
        }
        */
         for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (board[i][j] == 'O') board[i][j] = 'X';
                if (board[i][j] == '$') board[i][j] = 'O';
            }
        }
        
    }
    
    void BFS(vector<vector<char>>&board, int i, int j) {
        // so hang: 
        int m= board.size();
        // so cot;
        int n=board[0].size();
        /*
        anh xa mang 2 chieu m hang, n cot toi mang 1 chieu
        a[i,j] => t (i*n+j = t)
        a[i+1,j] => t+n
        a[i-1,j] => t-n
        a[i,j+1] => t+1
        a[i,j-1] => t-1
        */
    
        int t = i*n+j; // i=t/n, j=t%n
        board[i][j]='$';
        queue<int> q;
        q.push(t);
        
        while(!q.empty()) {
            int t = q.front(); q.pop();
            int x = t/n, y=t%n;
            // x+1 <= m-1
            if (x<=m-2 && board[x+1][y]=='O') {board[x+1][y]='$'; q.push(t+n);}
            // x-1>=0
            if (x>=1 && board[x-1][y]=='O') {board[x-1][y]='$'; q.push(t-n);}
            //y+1 <= n-1
            if (y<=n-2 && board[x][y+1]=='O') {board[x][y+1]='$'; q.push(t+1);}
            //y-1>=0
            if (y>=1 && board[x][y-1]=='O') {board[x][y-1]='$'; q.push(t-1);}
        }
    }
#endif

#if 0    
    // sample solution
    void solve(vector<vector<char>>& board) {
        if (board.empty() || board[0].empty()) return;
        int m = board.size(), n = board[0].size();
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != 0 && i != m - 1 && j != 0 && j != n - 1) continue;
                if (board[i][j] != 'O') continue;
                board[i][j] = '$';
                queue<int> q{{i * n + j}};
                while (!q.empty()) {
                    int t = q.front(), x = t / n, y = t % n; q.pop();
                    if (x >= 1 && board[x - 1][y] == 'O') {board[x - 1][y] = '$'; q.push(t - n);}
                    if (x < m - 1 && board[x + 1][y] == 'O') {board[x + 1][y] = '$'; q.push(t + n);}
                    if (y >= 1 && board[x][y - 1] == 'O') {board[x][y - 1] = '$'; q.push(t - 1);}
                    if (y < n - 1 && board[x][y + 1] == 'O') {board[x][y + 1] = '$'; q.push(t + 1);}
                }
            }
        }
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (board[i][j] == 'O') board[i][j] = 'X';
                if (board[i][j] == '$') board[i][j] = 'O';
            }
        }
    }
#endif
    
};