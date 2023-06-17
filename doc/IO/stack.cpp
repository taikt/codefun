#include <bits/stdc++.h>
using namespace std;

int main() {
    string xx = "abcdefgh";
    stack<char> st;
    for (int i = 0; i < xx.length(); i++) {
        st.push(xx[i]);
    }

    while (!st.empty()) {
        char x = st.top();
        st.pop();
        cout<<x<<" ";
    }

    return 0;
}