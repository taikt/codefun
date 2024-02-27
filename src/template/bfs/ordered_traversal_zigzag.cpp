// https://leetcode.com/problems/binary-tree-zigzag-level-order-traversal/description/

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
 // use BFS method
class Solution {
public:
    vector<vector<int>> zigzagLevelOrder(TreeNode* root) {
       
        vector<vector<int>> out;
        vector<int> currentLevel;
        //deque<TreeNode*> queue;
        queue<TreeNode*> queue;
        int alter = 1;
        int length =0;
        // for dequeue, use below
        //if (root) queue.push_back(root);
        if (root) queue.push(root);
        
        TreeNode* currentNode;
        
        while (!queue.empty()) {
            length = queue.size();
                 
            for (int i=0; i< length; i++) {           
                // for dequeue, use below
                //currentNode = queue.front(); queue.pop_front();     
                currentNode = queue.front(); queue.pop();
                currentLevel.push_back(currentNode->val);
                // for dequeue, use below       
                //if (currentNode->left) queue.push_back(currentNode->left);
                //if (currentNode->right) queue.push_back(currentNode->right);   
                
                if (currentNode->left) queue.push(currentNode->left);
                if (currentNode->right) queue.push(currentNode->right);      
            }
            // reverse elements of current level alternatively by next level
            if (alter < 0) 
                reverse(currentLevel.begin(),currentLevel.end());
            alter = (-1)*alter;
            out.push_back(currentLevel);
            currentLevel.clear();
            
        }
        return out;
    }
};