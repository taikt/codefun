#include "myhandler.h"
#include <iostream>
#include <string>
using namespace std;

void myhandler::handleMessage(const std::shared_ptr<Message>& msg) {
    switch (msg->what) {
        case TEST1:
            cout << "receive test1\n";
            break;
        case TEST2:
            cout << "receive test2\n";
            break;
        default:
            break;

    }

}