// std::map
// STL::map internal implementation is Balanced Red-Black Tree (a type of BST-Binary Search Tree)
// Delete, insert, search have complexity O(logn)
// https://stackoverflow.com/questions/2196995/is-there-any-advantage-of-using-map-over-unordered-map-in-case-of-trivial-keys

#include <cstdio>
#include <map>
#include <string>
using namespace std;

int main() {
	map<string,int> mapper;
	map<string,int>::iterator it;

 	//                       (grace,75) 
    //           (billy,69)               (martin,81)   
    //     (andy,80)   (felix,82)    (john,78)  (steven,77)
    // Hoat dong insert 
	mapper["chomas"] = 4;
	mapper["hin"] = 3;
	mapper["ainner"] = 7;
	mapper["filer"] = 7;
	mapper["kinner"] = 8;

	// Khong can quan tam thu tu duyet cay In-order, pre-oder hoac post-oder 
	// vi cay duoc re-rotate cay sau moi buoc insert de dam bao cay can bang
	// Voi BST, thoi gian duyet cay co the O(n) in worst case
	// Voi RBT, thoi gian duyet cay la O(logn) - chieu cao cua cay O(log(n))
	for (it = mapper.begin(); it != mapper.end(); it++)
	{
		printf("%s %d\n", ((string)it->first).c_str(), it->second);
	}

	it = mapper.find("filer"); // O(logn)
	if (it != mapper.end()) 
		printf("Found filter:%d\n",it->second);
	else printf("Not found filter\n");

	return 0;
}