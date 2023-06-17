#include <iostream>
#include <string>
#include <memory> // for smart pointer
using namespace std;

class Dog {
	string _name;
public:
	Dog(string name) {
		printf("dog constructor: named Dog\n");
		_name = name;
	}
	Dog() {
		printf("dog constructor: nameless dog\n");
		_name = "nameless";
	}
	~Dog() {
		printf("dog is destroyed\n");
	}
	void bark() {
		printf("dog name=%s\n", _name.c_str());
	}
};

void foo() {
	// Dog* p = new Dog("gunner");
	// p->bark();
	// delete p;
	
	// or use a shared poiter
	// 1. gunner is created on heap memory
	// 2. p is created
	shared_ptr<Dog> p(new Dog("gunner")); // count =1
	printf("count=%d\n", p.use_count());

	{
		shared_ptr<Dog> p2=p; // count = 2
		p2->bark();
		printf("count=%d\n", p2.use_count());
	}
	// count = 1
	printf("count=%d\n", p.use_count());
	p->bark();
	
} // count =0

int main() {
	foo();

	// wrong case:
	// when p go out scope, dog("tank") is deleted
	// when p2 go out scope, dog("tank") is deleted again => undefined behavior
	// d is raw pointer
	// suggestion: raw pointer should not used again successively by two shared pointers
	// correct way: when object is created, immediately assigned to shared pointer
	// like shared_ptr<Dog> p(new Dog("gunner")); 
	Dog* d = new Dog("tank");
	shared_ptr<Dog> p(d); // p.use_count() = 1
	//shared_ptr<Dog> p2(d); // wrong, p2.use_count() = 1
	shared_ptr<Dog> p2 = p; // is ok
	p->bark();
	p2->bark();
	

	// prefer way: mapping an object with shared pointer
	// 1. hail is created on heap memory
	// 2. p is created
	// 3. if hail is created sucessfully, p3 is fail, it delete 'hail' automatically
	// it cannot be done in this case: 
	// shared_ptr<Dog> p(new Dog("gunner"));
	shared_ptr<Dog> p3 = make_shared<Dog>("hail"); // faster and safer
	p3->bark();

}
