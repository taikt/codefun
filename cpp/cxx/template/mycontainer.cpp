// http://www.cplusplus.com/doc/oldtutorial/templates/
// template specialization

#include <cstdio>
#include <cstdlib>
using namespace std;

// 
// template <class identifier> class_declaration;
// template <typename identifier> class_declaration;
// no difference between keyword class or typename

// class template
template <class T> // template <typename T>
class mycontainer {
	T a;
public:
	mycontainer(T arg) {
		a=arg;
	}
	T increase();
};

template <class T>
T mycontainer<T>::increase() {
	a++;
	return a;
}


// class template specialization
template <>
class mycontainer <char> {
	char a;
public:
	mycontainer(char arg) {
		a = arg;
	}
	char uppercase() {
		a += 'A' - 'a';
		return a;
	}
};


int main() {
	mycontainer<int> myint(4);
	mycontainer<char> mychar('c');

	printf("%d\n",myint.increase());
	printf("%c\n",mychar.uppercase());

	return 0;
}