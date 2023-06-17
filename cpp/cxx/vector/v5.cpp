// Vector of strings
// Doc file input.txt va in ra cac phan tu

#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <fstream> // ifstream
#include <sstream> // stringstream
#include <iterator> // istream_iterator

using namespace std;

int main() {
	string filename = "input2.txt";

	ifstream f1(filename.c_str());

	if (!f1.is_open()) {
		printf("fail to open file\n");
		exit(0);
	}

	string line;

	while (getline(f1,line)) {		
		//printf("%s\n",line.c_str());
		stringstream st(line);
		istream_iterator<string> it(st);
		istream_iterator<string> end;
		vector<string> vt(it,end); //vt luu moi line cua file

		//printf("%s\n",vt[1].c_str());
		for (vector<string>::iterator it=vt.begin(); it != vt.end(); it++) {
			printf(" %s",(*it).c_str());
			
		}
		printf("\n");

	}

	f1.close();

	return 0;
}