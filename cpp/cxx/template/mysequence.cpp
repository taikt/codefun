// http://www.cplusplus.com/doc/oldtutorial/templates/
// non-type parameters for template

#include <cstdio>
#include <cstdlib>
using namespace std;

// 
// template <class identifier> class_declaration;
// template <typename identifier> class_declaration;
// no difference between keyword class or typename

// class template
template <class T, int N> // template <typename T>
class mysequence {
	T arr[N];
public:
	void setValue(int index, T value);
	T getValue(int index);
};

template <class T, int N>
void mysequence<T,N>::setValue(int index, T value) {
	arr[index] = value;
}

template <class T, int N>
T mysequence<T,N>::getValue(int index) {
	return arr[index];
}

int main() {
	mysequence<int,5> obint;
	mysequence<float,5> obfloat;

	obint.setValue(0,12);
	printf("%d\n",obint.getValue(0));

	obfloat.setValue(0,12.4);
	printf("%f\n",obfloat.getValue(0));

	return 0;
}