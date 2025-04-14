// http://stackoverflow.com/questions/5038895/does-stdsort-implement-quicksort
/*
std::sort is most likely to use QuickSort,
or at least a variation over QuickSort called IntroSort, which "degenerates" to HeapSort when the recursion goes too deep.

Complexity: O(N log(N)) comparisons
*/

//#include <iostream>     // std::cout
//#include <algorithm>    // std::sort
//#include <vector>       // std::vector
#include <bits/stdc++.h>
using namespace std;

bool myfunction (int i,int j) { return (i<j); }

struct myclass {
  bool operator() (int i,int j) { return (i<j);}
} myobject;

int main () {
  int myints[] = {32,71,12,45,26,80,53,33};
  std::vector<int> myvector (myints, myints+8);               // 32 71 12 45 26 80 53 33

  // using default comparison (operator <):
  //std::sort (myvector.begin(), myvector.begin()+4);           //(12 32 45 71)26 80 53 33

  // using function as comp
  //std::sort (myvector.begin()+4, myvector.end(), myfunction); // 12 32 45 71(26 33 53 80)

  // using lambda
  sort(myvector.begin(), myvector.end(), [](int a, int b){return a < b;}); // sap xep tang dan
  //sort(myvector.begin(), myvector.end(), [](int a, int b){return a > b;}); // sap xep giam dan

  // using object as comp
  //std::sort (myvector.begin(), myvector.end(), myobject);     //(12 26 32 33 45 53 71 80)

  // print out content:
  cout << "myvector contains:";
  for (vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    cout << ' ' << *it;
  cout << '\n';


  // sap xep mang 2 chieu [a,b] theo tu tu tang dan hoac giam dan chi so a hoac b
  // [2,5], [1,6], [4,9], [5,8] => [1,6], [2,5], [4,9], [5,8]
  vector<vector<int>> vector2D = {{2,5},{1,6},{4,9},{5,8}}; // vector2D[0]={2,5}, vector2D[1]={1,6}
  sort(vector2D.begin(), vector2D.end(), [](vector<int>& a, vector<int>& b){return a[1] < b[1];}); //sap xep theo chi so 1 a[0]<b[0], hoac chi so 2 a[1] <b[1]
  for(auto x:vector2D) {
    cout<<x[0]<<","<<x[1]<<'\n';
  }

  return 0;
}
