// Example program
#include <iostream> //std::cout
#include <utility> // std::pair, std::make_pair
#include <string> //std::string
#include <vector>

using namespace std;

typedef vector<int> vi;
typedef vector<vi> vii;


int main()
{

    vector<int> dist(3,200);

    for (vector<int>::iterator it = dist.begin(); it != dist.end(); ++it)
        cout <<' '<<*it;
    cout<<endl;

    dist[0]=2;
    dist[2]=3;

    for (vector<int>::iterator it = dist.begin(); it != dist.end(); ++it)
        cout <<' '<<*it;
    cout<<endl;

    //vii AdjList;
    //AdjList.assign(4,vi());
    vector<vector<int>> AdjList(4,vector<int>());

    AdjList[0].push_back(2); // 0-> {2, 4, 5}
    AdjList[0].push_back(4);
    AdjList[0].push_back(5);

    AdjList[1].push_back(6); // 1-> {6, 7}
    AdjList[1].push_back(7);

    AdjList[2].push_back(1); // 2-> {1, 9}
    AdjList[2].push_back(9);

    AdjList[3].push_back(10); // 3-> {10}

    cout<<AdjList[3][0]<<endl;

    vector<vector<int>> AdjList2 = {{2,3,5},{4,7}};
    cout<<AdjList2[0][1]<<endl;

    return 0;
}

