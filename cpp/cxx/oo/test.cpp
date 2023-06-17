
#include <iostream>
using namespace std;

//base class
class shape {
	public:
		void setWidth(int w) {
			width = w;
		}
		void setHigh(int h) {
			high = h;
		}
	
	protected:
		int width;
		int high;
};

// devided class
class square : public shape {
	public:
		int getArea() {
			return (width * high);
		}
};

int main(void)
{
	square sq;
	sq.setWidth(2);
	sq.setHigh(3);
	cout <<"area is: " <<sq.getArea()<<endl;
	
	return 0;
}


