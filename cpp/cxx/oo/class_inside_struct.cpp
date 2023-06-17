//is it possible to make class inside struct?

#include <cstdio>

class test {
private:
	int x;

public:
	
	test(int z) {
		x = z;
	}

	int square(int i) {
		return (i*i);
	}

};


struct table {
	test te;
	int i;
};

struct table tb;

int main() {
	test te(2);

	

	return 0;
}