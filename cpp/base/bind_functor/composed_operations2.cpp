#include <iostream>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include <map>


//#include <boost/function.hpp>
//#include <boost/bind.hpp>opBase
using namespace std;

// g++ binder_template2.cpp -std=c++11
// or g++ binder_template2.cpp -lboost_system
class Session
{
public:
    void accepterHandler(int err) {
        cout<<"[accepterHandler] err="<<err<<"\n";
    }

    void connectHandler(int err) {
        cout<<"[connectHandler] err="<<err<<"\n";
    }

    void f2(int a,int b) {
        cout<<"[f2] a="<<a<<" ,b="<<b<<"\n";
    }
};


template<typename Handler>
class acceptOperation {
public:
    acceptOperation(Handler h)
        :handler_(h)
    {}

    void perform(int& err) {
        cout<<"perform\n";
    }

    void complete(int err) {
        cout<<"complete, callback handler\n";
        this->handler_(err);
    }
private:
    Handler handler_;
};


class opBase {
public:
    typedef void(*complete_func_type)(opBase*, int& err);
    typedef void(*perform_func_type)(opBase*, int& err);
    opBase(perform_func_type perform_func, complete_func_type complete_func):
        perform_func_(perform_func), complete_func_(complete_func)
        {}
    void perform(int err) {
        return perform_func_(this,err);
    }

    void complete(int err) {
        return complete_func_(this,err);
    }

    std::shared_ptr<opBase> next;

private:
    complete_func_type complete_func_;
    perform_func_type perform_func_;
};

template<typename Operation>
class Op: public opBase {
public:
    Op(Operation operation)
        : opBase(&Op<Operation>::do_perform, &Op<Operation>::do_complete),
        operation_(operation)
    {}

    static void do_perform(opBase* base, int& err) {
        return static_cast<Op<Operation>*>(base)->operation_.perform(err);
    }

    static void do_complete(opBase* base, int& err) {
        return static_cast<Op<Operation>*>(base)->operation_.complete(err);
    }

public:
    Operation operation_;
};


template<typename Descriptor>
class OpsQueue {
public:
    OpsQueue() {}

    template<typename Operation>
    bool enqueueOp(std::shared_ptr<Op<Operation>>& op, Descriptor description) {
        cout<<"enqueueOp: description="<<description<<"\n";

        //opsMap_[description]->complete(3);
        if (opsMap_.count(description)) {
            cout<<"find existing entry of descrition:"<<description<<"\n";
            std::shared_ptr<opBase> tempOp = opsMap_[description];
            while (tempOp->next) {
                tempOp = tempOp->next;
            }
            tempOp->next = op;

        } else {
            cout<<"add new entry of descrition:"<<description<<"\n";
            opsMap_[description] = op;
        }
    }

    void getOps() {
        cout<<"getOps\n";
        std::shared_ptr<opBase> tempOp;
        for (const auto& x: opsMap_) {
            cout<<"key:"<<x.first<<"\n";
            //x.second->complete(2);
            tempOp = x.second;
            tempOp->complete(2);
            while(tempOp->next) {
                tempOp = tempOp->next;
                tempOp->complete(2);
            }
        }
    }

private:
    std::map<int, std::shared_ptr<opBase>> opsMap_;
};

class Reactor {
public:
    Reactor() {}

    template<typename Operation>
    void startReadOp(int description, Operation op) {
        //op.complete(3);
        std::shared_ptr<Op<Operation>> op_ptr = std::make_shared<Op<Operation>>(op);
        //TODO: add to queue this op
        // another thread get op from queue to handle independently
        //op_ptr->perform(2);
        //op_ptr->complete(3);
        readQueue_.enqueueOp(op_ptr, description);
    }

    void readOps() {
        cout<<"readOps\n";
        readQueue_.getOps();
    }

private:
    OpsQueue<int> readQueue_;
};

class Acceptor {
public:
    Acceptor(Reactor& react)
        :react_(react)
        {}

    template<typename Handler>
    void asyncAccept(int socket, Handler h) {
        react_.startReadOp(socket, acceptOperation<Handler>(h));
    }

private:
    Reactor& react_;
};


int main() {
    Reactor reactor_;
    Session sess_;
    Acceptor acc(reactor_);
    acc.asyncAccept(1,std::bind(&Session::accepterHandler, &sess_, std::placeholders::_1));
    acc.asyncAccept(2,std::bind(&Session::connectHandler, &sess_, std::placeholders::_1));
    acc.asyncAccept(1,std::bind(&Session::connectHandler, &sess_, std::placeholders::_1));

    reactor_.readOps();

    return 0;
}

