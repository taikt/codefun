#include "car.h"

int car::totalcount = 0;
car::car() :car(0)
{
	cout << "car()" << endl;
	/*
	++totalcount;
	cout << "car()" << "\n";
	fuel = 0;
	speed = 0;
	passengers = 0;
	*/
}

car::car(float amount) :car(amount, 0)
{
	cout << "car(float)" << endl;
	/*
	++totalcount;
	fuel = amount;
	speed = 0;
	passengers = 0;
	*/
}

// chi tao constructor code tai 1 vi tri de tranh mistake,
// nhung constructor khac se delegate toi constructor nay
car::car(float amount, int pass)
{
	cout << "car(float,int)" << endl;
	++totalcount;
	fuel = amount;
	speed = 0;
	passengers = pass;

}

car::~car()
{
}
