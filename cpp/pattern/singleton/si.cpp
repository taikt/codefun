#include <cstdio>
using namespace std;


class singleton {
	public:
		static singleton* instance() {
			if (_instance == 0)
			{
				_instance = new singleton;
			}
			return _instance;
		}

		singleton(){
			printf("init constructor\n");
		}

		static singleton* _instance;
};

singleton* singleton::_instance = 0; // can khai bao _instance la bien static

int main() {
	singleton* si = singleton::instance();

	return 0;
}