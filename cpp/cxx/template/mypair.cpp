// http://www.cplusplus.com/doc/oldtutorial/templates/
// class templates

#include <cstdio>
#include <cstdlib>
using namespace std;

// 
// template <class identifier> class_declaration;
// template <typename identifier> class_declaration;
// no difference between keyword class or typename

// class template
template <class T> // template <typename T>
class mypair {
	T a,b;
public:
	mypair(T first, T second) {
		a=first; b=second;
	}
	T getMax1();
};

template <class T>
T mypair<T>::getMax1() {
	return (a>b?a:b);
}

int main() {
	mypair<int> ob(100,75);

	printf("max=%d\n",ob.getMax1());

	return 0;
}