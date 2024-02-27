// https://leetcode.com/problems/binary-tree-level-order-traversal/description/

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
/*
    duyet cay boi BFS
*/
class Solution {
public:
    vector<vector<int>> levelOrder(TreeNode* root) {
        vector<vector<int>> out;
        vector<int> currentLevel;
        queue<TreeNode*> q;
        if (root)
            q.push(root);
        while (!q.empty()) {
            // tong so phan tu o level hien tai
            int current_length = q.size();
            
            for (int i=0; i< current_length; i++) {
                TreeNode* node = q.front(); q.pop();
                // add phan tu o level hien tai
                currentLevel.push_back(node->val);
                // add vao queue cho level tiep theo
                if (node->left) q.push(node->left);
                if (node->right) q.push(node->right);
            }
            // dua cac gia tri level hien tai vao output
            out.push_back(currentLevel);
            // reinit cac phan tu level hien tai la empty
            currentLevel.clear();
        }
        
        return out;
    }
};
