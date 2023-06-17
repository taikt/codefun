// https://thispointer.com/c-how-to-read-a-file-line-by-line-into-a-vector/

#include <bits/stdc++.h>
using namespace std;

bool iterateFile(string file, function<void (const string &)> callback) {
    ifstream in(file.c_str());
    if (!in) return false;//not open file
    string str;

    while (getline(in,str)) {
        callback(str);
    }

    in.close();
    return true;
}

int main() {
    vector<string> v;
    bool res = iterateFile("string_line.inp",[&](const string& str) {
                            v.push_back(str);
                           });

    if (res) {
        for (auto a:v)
            cout<<a<<"\n";
    }

    return 0;
}
