// 22.12.2016, Tai<taitrananhvn@gmail.com>

// sort algorithm example
// http://www.cplusplus.com/reference/algorithm/sort/
// http://stackoverflow.com/questions/9025084/sorting-a-vector-in-descending-order
// http://stackoverflow.com/questions/5038895/does-stdsort-implement-quicksort
// STL sort uses quicksort algorithm

#include <iostream>     // std::cout, std::greater
#include <algorithm>    // std::sort
#include <vector>       // std::vector
#include <cstdio>
using namespace std;

int main () {
  int myints[] = {32,71,12,45,26,80,53,33};
  vector<int> myvector (myints, myints+8);               // 32 71 12 45 26 80 53 33

/*
  sort (myvector.begin(), myvector.end()); // 12 26 32 33 45 53 71 80

  //sort(myvector.begin(), myvector.end(), greater<int>()); // 80 71 53 45 33 32 26 12

  // print out content:
  printf("myvector contains:\n");
  for (vector<int>::iterator it=myvector.begin(); it!=myvector.end(); ++it)
    printf("%d ", *it);
    printf("\n");
*/
  sort(&myints[0],&myints[0]+8);
  // sort(&myints[0],&myints[0]+8,greater<int>());
  for (int i=0; i<8; i++)
    printf("%d ",myints[i]);

  return 0;
}