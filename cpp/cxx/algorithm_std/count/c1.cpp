// cout: dem so xuat hien cua mot element trong mang hoac container (vector)
// http://www.cplusplus.com/reference/algorithm/count/?kw=count
#include <algorithm>
#include <vector>
#include <cstdio>
using namespace std;


int main() {
	int arr[] = {1,3,4,6,3};
	int x = count(arr,arr+5,3); // dem xuat hien thanh phan 3 trong mang

	printf("%d\n",x);

	vector<int> v(arr, arr+5); // dem xuat hien thanh phan 3 trong vector
	int y = count(v.begin(), v.end(),3);
	printf("%d\n",y);

	return 0;
}