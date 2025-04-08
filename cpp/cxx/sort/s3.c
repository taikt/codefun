// 22.12.2016, Tai<taitrananhvn@gmail.com>

// quicksort
// select a pilot (commonly a[0] or a[end])
// swap to move all element that < pilot to left side of pilot
// all elements that > pilot to right side of pilot
// example:
// a[0] a[1] a[2] a[3] a[5] a[6]
// select pilot = a[0], suppose a[3], a[6] < pilot, other > pilot
// patition the array to become
// a[3] a[6] a[0] a[1] a[2] => return index = 2 (a[0])
// recursively: quicksort(0,index-1) and quicksort(index+1,end)  
// Complexity: O(nlogn), https://www.cise.ufl.edu/class/cot3100fa07/quicksort_analysis.pdf

#include <stdio.h>


quicksort(int a[], int left, int right)
{
	int index;
	if (left < right)
	{
		index = patition(a, left, right);
		quicksort(a,left,index-1);
		quicksort(a,index+1,right);
	}
}

//void swap(int* a, int* b);
#if 0
int patition(int a[], int left, int right)
{
	int pilot, index, i;
	
	// select pilot as the first element
	pilot = a[left];
	index = left+1;
	// idea: pilot=a[0],a[1], a[2],..,a[index], a[index+1],
	// a[i] >= pilot, i>=index
	// a[i] < pilot, i <=index -1;
	// finally swap a[0] and a[index-1]
	// return (index -1) as index of pilot.
	for (i = left+1; i<= right; i++)
		if (a[i] < pilot)
		{
			swap(&a[i],&a[index]);
			index++;
		}
	swap(&a[index-1],&a[left]);
	return (index-1);
}
#endif 
int patition(int a[], int left, int right)
{
	int pilot, index, i;
	
	// select pilot as the last element
	pilot = a[right];
	index = left;
	// idea: a[0],a[1], a[2],..,a[index], a[index+1],...,a[right]=pilot,
	// a[i] >= pilot, i>=index
	// a[i] < pilot, i <=index -1;
	// finally swap a[right] and a[index]
	// return (index) as index of pilot.
	for (i = left; i<= right-1; i++)
		if (a[i] < pilot)
		{
			//swap(&a[i],&a[index]);
			swap(a,i,index);
			index++;
		}
	//swap(&a[index],&a[right]);
	swap(a,index,right);
	return (index);
}
#if 0
void swap(int* a, int* b)
{
	// first way of swap
	int x;
	x = *a;
	*a = *b;
	*b = x;
}
#endif
void swap(int a[],int i, int j)
{
	// second way of swap
	int temp;
	temp = a[i];
	a[i] = a[j];
	a[j] = temp;
}

int main()
{
	int a[] = {3,1,2,6,8,9, 4, 2, 15, 14, 5};
	int i;
	
	quicksort(a,0,(sizeof a/sizeof(int) - 1));
	for (i =0; i< sizeof a/sizeof(int); i++)
		printf("%d ",a[i]);
	printf("\n");
	
	return 0;
}