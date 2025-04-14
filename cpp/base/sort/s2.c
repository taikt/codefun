// 22.12.2016, Tai<taitrananhvn@gmail.com>

// O(n^2): bubble sort, selection sort, insertion sort
// O(n*log(n)): merge sort, quick sort
// gcc sorting.c -std=c99 
// https://www.cs.cmu.edu/~adamchik/15-121/lectures/Sorting%20Algorithms/sorting.html
// http://visualgo.net/sorting
// http://geeksquiz.com/merge-sort/
// https://www.hackerearth.com/notes/sorting-code-monk/
#include <stdio.h>


void bubbleSort(int ar[], int len)
{
  int i, j,temp;
  for (i = (len - 1);i >= 0; i--)
   {
      for (j = 1; j <= i; j++)
      {
        if (ar[j-1] > ar[j])
        {
              temp = ar[j-1];
              ar[j-1] = ar[j];
              ar[j] = temp;
		} 
	  } 
   } 
}

// 9 3 2 4 8 5
// 3 9 2 4 8 5 -> 2 9 3 4 8 5 -> 
// xet tai a[len-1]
// tim phan tu max (duyet tu 0 -> len-2), gan max vao a[len-1]
// xet tai a[len-2]
// tim phan tu max (duyet tu 0 -> len-3), gan max vao a[len-2]
// ...
// tai moi vi tri tim max: n lap, n lenh gan
void buble2Sort(int ar[], int len)
{
	int i, j, temp;
	for (i= 0; i < len; i++)
	{
		for (j=i+1; j< len; j++)
			if (ar[i]>ar[j])
			{
			   temp = ar[i];
			   ar[i] = ar[j];
			   ar[j] = temp;
			}
	}
}

// 9 3 2 4 8 5
// tim min tai a[0] trong 0-> len-1
// tim min tai a[1] trong 1-> len-1
// min= 2 => 2 3 9 3 8 5 
// tai moi vi tri tim max: n lap(de tim vtri min), 1 lenh gan (a[min] va tri tri tim max))
void selectionSort(int ar[], int len){
  for (int i = 0; i<len-1; i++)
   {
      int min = i;
      for (int j = i+1; j<len; j++)
      if (ar[j]<ar[min]) min = j;
      int temp = ar[i];
      ar[i] = ar[min];
      ar[min] = temp;
   } 
}


void insertionSort(int ar[], int len)
{
   for (int i=1; i < len; i++)
   {
      int index = ar[i]; int j = i;
      while (j > 0 && ar[j-1] > index)
      {
           ar[j] = ar[j-1];
           j--;
      }
      ar[j] = index;
   } 
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 =  r - m;
 
    /* create temp arrays */
    int L[n1], R[n2];
 
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1+ j];
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there
       are any */
    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there
       are any */
    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}
 
/* l is for left index and r is right index of the
   sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l+(r-l)/2;
 
        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m+1, r);
 
        merge(arr, l, m, r);
    }
}

// A utility function to swap two elements
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}
 
/* This function takes last element as pivot, places
   the pivot element at its correct position in sorted
    array, and places all smaller (smaller than pivot)
   to left of pivot and all greater elements to right
   of pivot */
// 2 15 14 7 9 -> expect: 2 7 9 15 14 -> 2 7 vs 15 14  
// pi=9, l=0, h=4
// i = -1; j=0-3 
// 13 2 1 7 9
// 
int partition (int arr[], int low, int high)
{
    int pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element
	//int i = low;  
 
    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
			swap(&arr[i], &arr[j]);
			//i++;
        }
    }
	swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}
 
/* The main function that implements QuickSort
 arr[] --> Array to be sorted,
  low  --> Starting index,
  high  --> Ending index */
void quickSort(int arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);
 
        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main()
{
	int arr[] = {1,2,4,3,6,8};
	int i=0, length;
	length = sizeof(arr)/sizeof(arr[0]);
	printf("length=%d\n",length);
	//bubbleSort(arr,length);
	buble2Sort(arr,length);
	//selectionSort(arr,length);
	//insertionSort(arr,length);
	//mergeSort(arr, 0, length - 1);
	//quickSort(arr, 0, length-1);
	for (i=0; i<length; i++)
	printf("%d  ", arr[i]);
	printf("\n");
	return 0;
}

