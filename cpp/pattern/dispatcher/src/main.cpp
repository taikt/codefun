#include "Dispatcher.h"
#include "Message.h"
#include <iostream>
#include <string>
using namespace std;

class myHandler : public Handler {
    void handleMessage(std::shared_ptr<Message>& msg) {
        cout<<"tai handle a message: id="<<msg->id<<", value:"<<msg->str<<"\n";
    }
};

int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();
    std::shared_ptr<Dispatcher> disp = std::make_shared<Dispatcher>(myHandler_);
    std::shared_ptr<Message> msg = std::make_shared<Message>(2,"abc");
    cout<<"send msg\n";
    disp->deliverMessage(msg);

    disp->deliverTask([=]{cout<<"hello Tai\n";});

    while (1) {}
    
    return 0;
}