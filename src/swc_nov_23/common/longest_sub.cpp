// https://leetcode.com/problems/longest-substring-without-repeating-characters/

// input: abcabcaaa
// out: 3
#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

#if 0
// Brute force
class Solution {
public:
    int lengthOfLongestSubstring(string s) {
        int max = 0;
        if (s.compare(" ") == 0) return 1;

        for (int i = 0; i < s.length(); i++) {
            for (int j = i; j < s.length(); j++) {
                if (check(i,j,s) && max < (j-i+1)) {
                    max = j - i + 1;
                }
            }
        }

        return max;
    }

    bool check(int i, int j, string& st) {
        set<char> s;
        for (int k = i; k <= j; k++) {
            if (s.count(st[k])) return false;
            s.insert(st[k]);
        }

        return true;
    }
};
#endif

// slide window
class Solution {
public:
    int lengthOfLongestSubstring(string st) {
        int max = 0;
        int l=0,r=0;
        if (st.size() == 0) return 0;
        set<char> s;

        while (l < st.length() && r < st.length()) {
           
            if (!s.count(st[r])) {
                s.insert(st[r]);
                if (max < r - l + 1) max = r-l+1;
                r++;
            } else {
                s.erase(st[l]);
                l++;
            }
        }
        return max;
    }
};

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    string line;
    getline(cin, line);
    Solution a;
    cout<<a.lengthOfLongestSubstring(line)<<"\n";
   


    return 0;
}