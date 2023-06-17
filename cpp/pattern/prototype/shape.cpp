#include <string>
#include <iostream>
using namespace std;

class shape {
public:
    int x;
    int y;
    //string color;
public:
    shape() {}
    shape(int _x, int _y)
    : x(_x),y(_y) {
    }
    // copy constructor
    shape(const shape& s) {
        x = s.x;
        y = s.y;
    }

    virtual ~shape() {}
    // prototype method
    virtual shape *clone() const = 0;
};

class rectangle : public shape {
public:
    int width;
    int height;

public:
    rectangle(int _width, int _heigh)
    : shape(_width,_heigh) {
        width = _width;
        height = _heigh;
    }
    rectangle(const rectangle& rec): shape(rec) {
        //shape(rec);
        width = rec.width;
        height = rec.height;
    }
    shape* clone() const override {
        return new rectangle(*this);
    }
};

int main() {
    shape* rec = new rectangle(10,20);
    //rec.width = 10;
    //rec.height = 20;

    //copy another rectangle
    shape* rec2 = rec->clone();
    cout<<"width="<<rec2->x<<",height"<<rec2->y;

    return 0;
}
