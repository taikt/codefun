#include "bits/stdc++.h"
using namespace std;

int main() {
    unordered_map<char,int> m;
    m['a'] = 1;
    m['b'] = 2;
    m['a'] = 3;
    cout<<m['a']; //3: update new value for existing key

    return 0;
}
