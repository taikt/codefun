// 22.12.2016, Tai<taitrananhvn@gmail.com>

// Heap sort implementation
// Complexity: O(nlogn)

// Heapsort su dung cau truc du lieu Binary heap
// Binary heap la 1 Binary tree voi cac thuoc tinh sau
// 1. La cay day du (tru node la - cac khoa sap xep tu trai truoc)
// 2. Hoac la Min Heap hoac Max Heap, voi Min Heap - node root la nho nhat
// thuoc tinh nay de qui dung cho cac node con lai.

// Cac hoat dong cua Min Heap
// 1. getmini()- O(1)
// 2. extractMin() - O(logn)
// 3. decreaseKey() - O(logn)
// 4. insert() - O(logn)
// 5. delete() - O(logn)

// Ung dung cua Binary Heap
// 1. Heapsort
// 2. Priority queue
// 3. Graph algorithms (shortest path Dijkstra, Prim's Minimum Spanning Tree)


//Heapsort
// 1. Xay dung Binary Max Heap tu du lieu input
// 2. Gia tri maximum la root cua tree. Thay the gia tri root voi node la cuoi cung
// Dong thoi giam size cua Heap 1 don vi, Heapify tai node root de duy tri thuoc tinh 
// Max Heap
// De qui Lap lai buoc 2 

// Heapify tai node i (duy tri thuoc tinh heap cho subtree voi root tai i)
// 1. Tim maximum item giua left_child i, righ_child i
// 2. Swap node i va maximum item
// 3. De qui Heapify tai maximum item

// Xay dung Binary Max Heap
// 1. For (i=[n/2], i--)
// Heapify(i)

// Refer
// http://quiz.geeksforgeeks.org/binary-heap/
// https://visualgo.net/heap

#include <cstdio>

void swap(int* x, int* y);

void Heapify(int arr[], int n, int i) {
	int max_item;
	int left, right;

	max_item = i;
	left = 2*i+1; right = 2*i+2;

	if (arr[max_item] < arr[left] && left < n)
		max_item = left;

	if (arr[max_item] < arr[right] && right < n)
		max_item = right;

	if (max_item != i) {
		swap(&arr[i],&arr[max_item]);
		Heapify(arr,n,max_item);
	}
}

void Heapsoft(int arr[], int n) {
	
	// Build Max Heap
	for (int i = n/2 -1; i >= 0; i--) {
		Heapify(arr,n,i);
	}

	// Swap the root with the last leaf item and Heapify the root
	for (int i=n -1; i >= 0; i--) {
		swap(&arr[0],&arr[i]);
		Heapify(arr,i,0);
	}
}

void swap(int* x, int* y) {
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

int main() {
	int arr[] = {2, 6, 3, 5, 10, 4,3,36,12};
	int size = sizeof(arr)/sizeof(arr[0]);
	
	Heapsoft(arr,size);
	printf("Array in ascending order\n");
	for (int i=0; i < size; i++)
		printf("%d  ", arr[i]);
	printf("\n");


	return 0;
}