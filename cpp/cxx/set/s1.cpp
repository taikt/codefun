// Set internal implementation is Red/Black Tree (a type of BST)
// STL:map stores <key,data>
// STL:set stores <key>
// Delete, insert, search have complexity O(logn)

#include <cstdio>
#include <set>
#include <string>
using namespace std;


int main()
{
	//                 (78) 
    //         (75)            (81)   
    //     (69)    (77)    (80)    (82)
	set<int> test;
	set<int>::iterator it;
	test.insert(78);
	test.insert(40);
	test.insert(30);
	test.insert(80);
	test.insert(31);

	for (it=test.begin(); it != test.end(); it++) {
		printf("%d\n",*it);
	}

	it = test.find(30); // O(logn)
	if (it != test.end()) 
		printf("Found 30 =%d\n",*it);
	else printf("Not found 30\n");
	

	return 0;
}