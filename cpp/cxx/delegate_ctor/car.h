#pragma once
using namespace std;
#include <iostream>

class car {
private:
	float fuel;
	float speed;
	int passengers;
	static int totalcount;
public:
	car();
	car(float amount);
	car(float amount, int pass);
	~car();
};
