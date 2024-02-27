#include <bits/stdc++.h>
using namespace std;

// input.txt
// 2 3 4 5 6 7 9
int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    
    vector<int> mystr;
    int word;
    string st;


    getline(cin,st);
    stringstream str(st);
    while (str>>word)
        mystr.push_back(word);

    for (auto i=0; i<mystr.size(); i++) {
        cout<<mystr[i]<<"\n";
    }


    return 0;
}