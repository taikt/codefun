// https://viblo.asia/p/gioi-thieu-mot-so-ham-tim-kiem-co-san-trong-stl-c-Do754OvLlM6

#include<bits/stdc++.h>
using namespace std;

int main() {
	int arr[] = {2,3,1,7,4,15,13};
	vector<int> vt(arr,arr+7);
	
	// 1 2 3 4 7 13 15
	sort(vt.begin(), vt.end());

	for (int i=0; i< vt.size(); i++)
		printf("%d ",vt[i]);

	printf("\n");

	// find pos in [l,r-1] that vt[pos]>=4
	// if cannot find -> return r
	cout<<lower_bound(vt.begin(),vt.end(),4)-vt.begin(); //pos=3

	// test

	return 0;
}