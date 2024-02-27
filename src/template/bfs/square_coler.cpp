// Taikt code
#include <bits/stdc++.h>

using namespace std;
const int maxn= 1000;
int h, w, a[maxn][maxn];
void solve() {
    cin >> h >> w; 
    char c;
    queue<pair<int,int>> q;
    for (int i = 0; i <h ; i++)
    for (int j = 0; j < w; j++) {
        cin >> c;
        if (c == '.') a[i][j] = 0;
        else if (c == '#') {
            a[i][j] = 1;
            q.push({i,j});
        }
    }
    
    int dx[] = {0,1,0,-1};
    int dy[] = {-1,0,1,0};
    int level = 0;
    while(!q.empty()) {
        int csize = q.size();
        int count = 0;
        level++;
        while (count < csize) {
            pair<int,int> cur = q.front();
            q.pop();
            int cx = cur.first, cy = cur.second;
            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i], ny = cy + dy[i];
                if (nx < 0 || nx >= h || ny <0 || ny >= w || (a[nx][ny] == 1)) continue;
                a[nx][ny] = 1;
                q.push({nx,ny});
            }
            count++;
        }
    }
    if (level != 0)cout<<level-1<<"\n";
    else cout<<"0";
    
    return;
}
int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    solve();
    return 0;
}

/*

#include <bits/stdc++.h>
using namespace std;

int row, col;
vector<vector<char>> grid(1000, vector<char>(1000)); 
queue<std::pair<int, int>> q;
int moveX[] = {0, 0, -1 , 1};
int moveY[] = {-1, 1, 0, 0};


bool check(int i, int j) {
    return (i >= 0 && i < row && j >= 0 && j < col && grid[i][j] == '.');
}

int bfs() {
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
          if (grid[i][j] == '#'){
            q.push(make_pair(i, j));
          }
        }
    }
    int op = -1;
    while (!q.empty())
    {
        int currentSize = q.size();
        for (int k = 0; k < currentSize; k++) 
        {
            pair<int, int> p = q.front();  // get the first element at the queue
            q.pop();
            for (int i = 0; i < 4; i++) {
                int u = p.first + moveX[i];
                int v = p.second + moveY[i];
                if (!check(u, v)) continue;
                grid[u][v] = '#';
                q.push({u, v});
            }
        }
      op++;
    }
    return op;
}


// Driver code
int main()
{
  cin >> row >> col;
  for(int i = 0 ; i < row; i++){
    for(int j = 0; j < col; j++){
        cin >> grid[i][j];
    }
  }

  cout << bfs();
}


*/