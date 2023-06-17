// istream_iterator example
// istream iterator are input iterators that read successive elements from 
// an input stream, such as cin
// http://www.cplusplus.com/reference/iterator/istream_iterator/?kw=istream_iterator

// ifstream is input stream class to operate on files
// http://www.cplusplus.com/reference/fstream/ifstream/?kw=ifstream

// check iterator example in vector part

#include <iostream>
#include <iterator> // istream_iterator
#include <cstdio>
using namespace std;

int main() {
	int vl1, vl2;
	printf("enter two values:\n");

	istream_iterator<int> eos; //end of stream iterator
	istream_iterator<int> iit(cin);

	if (iit != eos) vl1 = *iit;
	++iit;

	if (iit != eos) vl2 = *iit;

	printf("value1 =%d, value2 =%d\n",vl1, vl2);

	return 0;
}
