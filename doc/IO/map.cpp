#include <bits/stdc++.h>
using namespace std;

int main() {
    map<int,string> m; // map sap xep item theo ordered (balanced tree)
    // unordered_map: sap xep item khong theo order (hash)
    m[2] = "a";
    m[3] = "x";
    m[4] = "c";
    printf("loop 1: forward\n");
    for (auto it : m) {
        printf("%d %s\n", it.first, it.second.c_str());
        // 2 a
        // 3 x
        // 4 c
    }

    printf("loop 2: forward\n");
    for (auto it=m.begin(); it != m.end(); it++) {
        printf("%d %s\n", it->first, it->second.c_str());
        // 2 a
        // 3 x
        // 4 c
    }

    printf("loop 3: reverse\n");
    for (auto it=m.rbegin(); it != m.rend(); it++) {
        printf("%d %s\n", it->first, it->second.c_str());
        // 4 c
        // 3 x
        // 2 a
    }

    return 0;

}