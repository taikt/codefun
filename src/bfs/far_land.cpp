// https://leetcode.com/problems/as-far-from-land-as-possible/description/

class Solution {
    // BFS solution
    /*
    khởi tạo add các đỉnh land vào queue.
    tìm các đỉnh water lân cận đỉnh land, update khoảng cách tới đỉnh water = khoảng cách từ đỉnh land +1
    (đỉnh water có đường đi ngắn nhất xuất phát từ đỉnh land được duyệt qua bfs)
    bfs đảm bảo khoảng cách đỉnh water là nhỏ nhất vì gần với đỉnh land nhất, vì được duyệt trước
    
    */
public:
    int maxDistance(vector<vector<int>>& grid) {
		int locMax = 0;
		queue<pair<int, int>> q;
		for (int i = 0; i < grid.size(); ++i) {
			for (int j = 0; j < grid[i].size(); ++j) {
				if (grid[i][j] == 1) q.push({ i, j });
			}
		}

		while (!q.empty()) {
			pair<int, int> cur = q.front();
			q.pop();
			pair<int, int> dirs[4] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
			for (auto dir : dirs) {
				int nx = dir.first + cur.first;
				int ny = dir.second + cur.second;
				if (nx >= 0 && nx < grid.size() && ny >= 0 && ny < grid[0].size() && grid[nx][ny] == 0) {
					q.push({ nx, ny });
					grid[nx][ny] = grid[cur.first][cur.second] + 1;
					locMax = max(grid[nx][ny], locMax);
				}
			}
		}
		return (locMax > 0 ? locMax-1 : -1);
	}
};