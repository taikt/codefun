/*
    https://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
    https://www.geeksforgeeks.org/split-a-sentence-into-words-in-cpp/

    Input:
    accounts = [["John", "johnsmith@mail.com", "john00@mail.com"], ["John", "johnnybravo@mail.com"], ["John", "johnsmith@mail.com", "john_newyork@mail.com"], ["Mary", "mary@mail.com"]]
    Output: [["John", 'john00@mail.com', 'john_newyork@mail.com', 'johnsmith@mail.com'],  ["John", "johnnybravo@mail.com"], ["Mary", "mary@mail.com"]]

*/
#include <bits/stdc++.h>
using namespace std;

#define FOR(i,a,b) for(int i=a;i<b;i++)


int main() {

    int n=0;
    string st,word="";

    freopen("string_line.inp","r",stdin);
    freopen("string_line.out","w",stdout);

    scanf("%d\n",&n);
    vector<vector<string>> accounts(n);

    FOR(i,0,n) {
        getline(cin,st);
        word="";
#if 1
        for (auto x:st) {
            if (x==' ') {
                accounts[i].push_back(word);
                word="";
            } else word+=x;
        }
#endif // 0
#if 0
        FOR(j,0,st.length()) {
            if (st[j]==' ') {
                accounts[i].push_back(word);
                word="";
            } else word+=st[j];
        }
#endif // 0
        accounts[i].push_back(word);//end of st without a character ' '
    }

    FOR(i,0,n) {
        for(auto x:accounts[i])
            cout<<x<<" ";
        cout<<"\n";
    }


    return 0;
}
