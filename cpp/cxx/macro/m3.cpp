#include <iostream>
using namespace std;

void print_array(int *a)
{
	for (int i=0; i<10; i++)
		cout<<a[i]<<endl;
}

int main()
{
	// 1
#if 0	
	int *ip = new int[10];
	
	for (int i=0; i<10; i++)
	cout <<ip[i]<<endl;
	
	delete[] ip;
#endif 
	
	// 2
	int *ip2 = new int[10];
	for (int i=0; i<10; i++)
		ip2[i] = 10 + i;
	print_array(ip2);
	
	return 0;
}