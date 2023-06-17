using namespace std;
#include <exception>
#include <string>
#include <iostream>

// vi du tao 1 customized exception class
class Except: public std::exception {
int error_num;
std::string msg; // tin nhan loi khi hien thi exception
public:
    Except(const std::string& _msg, int err):
        error_num(err),
        msg(_msg)
    {
        cout<<"except"<<"\n";
    }
    ~Except() throw() {
        cout<<"~except"<<"\n";
    }
    const char* what() const noexcept{ // duoc ket thua tu std::exception
        return msg.c_str();
    }
    const int getError() const{
        return error_num;
    }

};

void myFunc() {
    throw(Except("my error",-5)); // trigger exception, can phai catch, otherwise chuong trinh bi terminated
}

int main() {
    try {
        myFunc();
    } catch (const Except& e) {
        cout<<e.what()<<"\nError numer:"<<e.getError()<<"\n"; // lay exception msg va error code
    }

    return 0;
}
