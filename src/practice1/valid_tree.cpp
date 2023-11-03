// https://leetcode.com/problems/graph-valid-tree/description/
class Solution {
public:
    bool validTree(int n, vector<vector<int>>& edges) {
        queue<int> q;
        int t;
        vector<unordered_set<int>> ed(n,unordered_set<int>());
        unordered_set<int> visited;
        for (auto x: edges) {
           ed[x[0]].insert(x[1]);
           ed[x[1]].insert(x[0]);
        }
        
        q.push(0);visited.insert(0);
        while (!q.empty()) {
            t = q.front(); q.pop();           
            for (auto x: ed[t]) {
                if (visited.count(x)) return 0;
                visited.insert(x);
                q.push(x);
                ed[x].erase(t);
            }
        }
        for (int i=0; i<n;i++) {
            if (!visited.count(i)) return 0;
        }
        
        return 1;
    }
};