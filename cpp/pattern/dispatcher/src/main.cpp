#include "Dispatcher.h"
#include "Message.h"
#include <iostream>
#include <string>
using namespace std;


int main() {
    std::shared_ptr<Dispatcher> disp = std::make_shared<Dispatcher>();
    std::shared_ptr<Message> msg = std::make_shared<Message>(2,"abc");
    cout<<"send msg\n";
    disp->deliverMessage(msg);

    disp->deliverTask([=]{cout<<"hello Tai\n";});

    while (1) {}
    
    return 0;
}