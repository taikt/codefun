#include <iostream>
using namespace std;

bool accept3() {
	int tries = 1;
	while (tries < 4) {
		cout << "do you want to process (y or n) \n";
		char answ = 0;
		cin >> answ;
		
		switch (answ) {
			case 'y':
				return true;
			case 'n':
				return false;
			default:
				cout <<"i dont understand\n";
				++tries;
		}
	}
	return false;
}

int main()
{
	accept3();
	
	return 0;
}