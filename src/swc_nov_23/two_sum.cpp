/*
//input
2 7 11 15
9
// output
0 1

//input
3 2 4
6
// output
1 2
*/
#include <bits/stdc++.h>
#include "log.hpp"
using namespace std;

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);

    vector<int> s_arr;
    int number;
    string line;
    

    getline(cin, line);
    stringstream str(line);

    while (str>>number) {
        s_arr.push_back(number);
    }

    cout<<s_arr;

    return 0;
}
