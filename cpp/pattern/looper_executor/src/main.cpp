#include "myhandler.h"
#include "SLLooper.h"
#include "Refbase.h"
#include <iostream>
#include <string>
using namespace std;


int main() {
    std::shared_ptr<SLLooper> looper = std::make_shared<SLLooper>();
    std::shared_ptr<myhandler> _h = std::make_shared<myhandler>(looper);
    std::shared_ptr<RefBase> msg = std::make_shared<RefBase>();
    msg->id = 2;
    cout<<"call exit looper\n";
    //looper->exit();
    _h->sendMessage(_h->obtainMessage(TEST1, msg));


    return 0;
}