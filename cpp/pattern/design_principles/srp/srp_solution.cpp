#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

class Diary
{
public:
    Diary(const string &title) : title_(title) {}
    void add(const string &entry)
    {
        static int cnt = 0;
        string entry_ = std::to_string(cnt++) + ":" + entry;
        entries.push_back(entry_);
    }

public:
    string title_;
    vector<string> entries;
};
/**
 * Move persistence functionality to its own class
 * @param  {c []} undefined : 
 */
class PersistencMgr
{
public:
    void save(Diary &dia, const string &filename)
    {
        ofstream ofs(filename);
        for (auto &s : dia.entries)
            ofs << s << "\n";
    }
};

int main()
{
    Diary dia{"my diary"};
    dia.add("line 1");
    dia.add("line 3");

    PersistencMgr perMgr;
    perMgr.save(dia, "diary.txt");

    return 0;
}