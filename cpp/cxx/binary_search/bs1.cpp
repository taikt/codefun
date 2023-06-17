#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
	int arr[] = {2,3,1,7,4,15,13};
	vector<int> vt(arr,arr+7);

	sort(vt.begin(), vt.end());

	for (int i=0; i< vt.size(); i++)
		printf("%d ",vt[i]);

	printf("\n");

	if (binary_search(vt.begin(), vt.end(), 8))
		printf("found 8\n");
	else printf("not found 8\n");

	// test

	return 0;
}