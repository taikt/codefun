#include <bits/stdc++.h>
using namespace std;

int main() {
    string xx = "abcdefgh";
    queue<char> q;
    for (int i = 0; i < xx.length(); i++) {
        q.push(xx[i]);
    }

    while (!q.empty()) {
        char x = q.front();
        q.pop();
        cout<<x<<" ";
    }

    return 0;
}