// https://www.geeksforgeeks.org/map-vs-unordered_map-c/
// https://stackoverflow.com/questions/40320455/comparing-unordered-map-vs-unordered-set/40320514
// https://www.geeksforgeeks.org/set-vs-unordered_set-c-stl/
// https://www.techiedelight.com/print-keys-values-map-cpp/

/*
	unordered_map: hash, map key and value, O(1), use when don't need ordered data
	unordered_set: hash, only contain key, O(1), use when don't need ordered data
	map: balancing BST, map key and value, O(logn), use when need ordered data
	set: balancing BST, only contain key, O(logn), use when need ordered data

*/
#include "bits/stdc++.h"
using namespace std;

void test_unordered_map() {
    unordered_map<int,int> m;
    // Mapping values to keys
    m[5] = 10;
    m[3] = 5;
    m[20] = 100;
    m[1] = 1;

#if 0
    if (m.count(3)) {
        printf("unordered_map: exist 3\n");
        m.erase(3);
    }

    if (m.count(3))
        printf("unordered_map: exist 3\n");
    else printf("unordered_map: doesn't exist 3\n");
#endif // 0
	 // Iterating the map and printing unordered values
	printf("test_unordered_map\n");
    for (auto i : m) {
        cout << i.first << " : " << i.second<< '\n';
    }
	printf("\n");
}

void test_map() {
	// Ordered map
    map<int, int> m;

    // Mapping values to keys
    m[5] = 10;
    m[3] = 5;
    m[20] = 100;
    m[1] = 1;

    // Iterating the map and printing ordered values
    printf("test_map\n");
    for (auto i : m) {
        cout << i.first << " : " << i.second << '\n';
    }
	printf("\n");

}

void test_unordered_set() {
    unordered_set<int> set;
    set.insert(3); //only store key
    set.insert(4);
    set.insert(1);

#if 0
    if (set.count(3)) {
        printf("set exist 3\n");
        set.erase(3);
    }

    if (set.count(3))
        printf("set exist 3\n");
    else printf("set doesn't exist 3\n");
#endif
	cout << "\ntest_unordered_set \n";
	set.erase(4);
	set.erase(7); // neu khong ton tai phan tu, cung khong sao
    for (auto it : set)
        cout << it << " ";
	printf("\n");

}



void test_set() {
	set<int> set;
    set.insert(3); //only store key
    set.insert(4);
    set.insert(1);
	cout << "\ntest_set \n";
    for (auto it : set)
        cout << it << " ";
	printf("\n");
}

int main() {
    //test_unordered_map();
	//test_map();
    test_unordered_set();
	//test_set();

    return 0;
}
