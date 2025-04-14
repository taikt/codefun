// vector of strings
using namespace std;
#include <stdio.h>
#include <vector>
#include <string>

template<typename T, int N>

T* size(T (&ra)[N]) {
	return ra+N;
}

int main() {
	char* ip[] = {"one","two","three"};

	//vector<string> v(ip,ip+3); //3: size of ip array
	vector<string> v(ip,size(ip)); 
    printf("size of ip:%d\n",size(ip));
	printf("%s\n",v[2].c_str());


	// the iterator constructor can also be used to construct from arrays:
  	int myints[] = {16,2,77,29};
  	vector<int> fifth (myints, myints + sizeof(myints) / sizeof(int) );

  	printf("%d\n",fifth[0]);
  	printf("The contents of fifth are:\n");
  	for (vector<int>::iterator it = fifth.begin(); it != fifth.end(); ++it)
    	printf(" %d",*it);
  	  	
  	printf("\n");

	return 0;
}
