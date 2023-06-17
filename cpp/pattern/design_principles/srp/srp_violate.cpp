/**
 * Single responsibility principle:
 * one class should do a specific job, not different jobs
 * @param  {# []} undefined : 
 */
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
    /**
     * Delare save here violates SCP
     * saving to persistence is a separate concern, should declared in another class such
     * as persitenceManager. it works with persistance stuff. like save, delete
     * @param  {string} filename : 
     */
    void save(const string &filename)
    {
        ofstream ofs(filename);
        for (auto &s : entries)
            ofs << s << "\n";
    }

private:
    string title_;
    vector<string> entries;
};

int main()
{
    Diary dia{"my diary"};
    dia.add("line 1");
    dia.add("line 2");
    dia.save("diary.txt");

    return 0;
}