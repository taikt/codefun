#include <bits/stdc++.h>
using namespace std;

template<typename T>
ostream& operator<<(ostream& out, vector<T>& vec) {
    if (vec.size() == 0)
    {
        out<<"{}\n";
        return out;
    }
    out<<"{";
    for (int i = 0; i < vec.size() - 1; i++) {
        out<<vec[i]<<", ";
    }
    out<<vec.back()<<"}\n";
    return out;
}

int main() {
    ios::sync_with_stdio(0);
    cin.tie(0);
    freopen("input.txt","r",stdin);
    //freopen("output.txt","w",stdout);
    string line;
    getline(cin, line);
    stringstream str(line);
    int num;
    vector<int> numarr;
    while (str>>num) {
        numarr.push_back(num);
    }
    int value;
    cin>>value;

    cout<<numarr;
    //sort(numarr.begin(), numarr.end(), greater<int>());
    sort(numarr.begin(), numarr.end()); // increasing
    cout<<numarr;
    cout<<"need to find:"<<value<<"\n";

    if (binary_search(numarr.begin(), numarr.end(), value))
    cout<<"found\n";
    else cout<<"not found\n";


    return 0;
}