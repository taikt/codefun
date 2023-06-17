// http://www.cplusplus.com/doc/oldtutorial/templates/
// function templates

#include <cstdio>
#include <cstdlib>
using namespace std;

// 
// template <class identifier> function_declaration;
// template <typename identifier> function_declaration;
// no difference between keyword class or typename

template <class T> // template <typename T>
T getMax1(T a, T b) {
	T rs;
	rs = (a>b)? a:b;
	return rs;
}

template <class T, class U>
T getMax2(T a, U b) {
	return ((a>b)? a:b);
}

int main() {
	int i=5, j=6;
	long l=10, m=11;
	printf("max=%d\n",getMax1<int>(i,j));
	printf("max=%d\n",getMax1<long>(l,m));
	printf("max=%d\n",getMax2<int,long>(i,l));

	return 0;
}