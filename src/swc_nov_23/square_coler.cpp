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