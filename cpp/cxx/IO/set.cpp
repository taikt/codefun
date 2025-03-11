//test duplicate insert
#include <bits/stdc++.h>
using namespace std;

int main() {
    set<vector<int>> ret;
    ret.insert({1,2,6});
    ret.insert({4,2,6});
    ret.insert({2,1,6});
    ret.insert({4,2,6}); //duplicate (no insert)

    return 0;
}
