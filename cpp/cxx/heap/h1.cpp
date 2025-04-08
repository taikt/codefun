// 22.12.2016, Tai<taitrananhvn@gmail.com>

// Cac hoat dong Binary Heap
// Refer
// http://quiz.geeksforgeeks.org/binary-heap/

#include <iostream>
#include <cstdio>
using namespace std;

void swap(int *x, int*y);

class MinHeap
{
	int *harr;
	int capa; //maximum size
	int heap_size;

public:
	//constructor
	MinHeap(int capa);

	//heapify a subtree
	void MinHeapify(int);

	int parent(int i) {return (i-1)/2;}

	int left(int i) {return (2*i+1);}

	int right(int i) {return (2*i+2);}

	int extractMin();

	void deleteKey(int i);

	void insertkey(int k);

	void print_heap();
};

MinHeap::MinHeap(int cap) {
	heap_size = 0;
	capa = cap;
	harr = new int[cap];
}

void MinHeap::insertkey(int k) {
	int i;

	if (heap_size == capa)
	{
		printf("overflow heap\n");
		return; 
	}

	heap_size++;
	harr[heap_size-1] = k;
	i = heap_size -1;

	while ((i !=0)  && (harr[i] < harr[parent(i)]))
	{
		swap(&harr[i],&harr[parent(i)]);
		i = parent(i);
	}

}

void MinHeap::print_heap() {
	for (int i = 0; i < heap_size; i++)
		printf("%d ",harr[i]);
	printf("\n");
}


void swap(int *x, int* y) {
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

int main()
{
	MinHeap h(10);
	h.insertkey(2);
	h.insertkey(5);
	h.insertkey(3);
	h.print_heap();

	return 0;

}