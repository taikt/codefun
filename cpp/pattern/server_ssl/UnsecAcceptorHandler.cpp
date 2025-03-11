#include "Reactor.h"
#include "UnsecAcceptorHandler.h"
#include "ReaderHandler.h"
#include <openssl/ssl.h>
#include <unordered_map>
using namespace std;

extern std::unordered_map<uint32_t, SSL*> ssl_map;

UnsecAcceptorHandler* UnsecAcceptorHandler::singleton_= nullptr;
extern std::shared_ptr<Dispatcher> taskDispatcher_;

UnsecAcceptorHandler* UnsecAcceptorHandler::getInstance()
{
   if (singleton_ == nullptr) {
		singleton_ = new UnsecAcceptorHandler();
   }
   return singleton_;
}

void UnsecAcceptorHandler::setReactor(EventNotifier* reactor) {
	reactor_ = static_cast<Reactor*>(reactor); // TODO check
}

void UnsecAcceptorHandler::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void UnsecAcceptorHandler::handleEvent(int sd,EventType et)
{
    struct sockaddr_in cli_addr;
    socklen_t          clilen;

    if (taskDispatcher_ == nullptr) {
        printf("task dispatcher is null\n");
        return;
    }

    if (et == ACCEPT_UNSECURED_EVENT) {
        printf("unsecured listen port:%d\n", (int) reactor_->unsecTcpListenPort->ListenFd);
        memset(&cli_addr, '0', sizeof(cli_addr));
        clilen = sizeof(cli_addr);

        if ((reactor_->unsecTcpConnectedPort = AcceptTcpConnection(reactor_->unsecTcpListenPort, &cli_addr, &clilen)) == NULL)
        {
            printf("AcceptTcpConnection Failed\n");
            
            return;
        }
        else {
            printf("handle unsecured connection for a client: fd=%d\n", (int)reactor_->unsecTcpConnectedPort->ConnectedFd);
            if (reactor_ != nullptr) {  
                printf("add new unsecured client to reactor: %d\n", (int)reactor_->unsecTcpConnectedPort->ConnectedFd);
                reactor_->addClient(reactor_->unsecTcpConnectedPort->ConnectedFd);
                reactor_->unsec_set.insert(reactor_->unsecTcpConnectedPort->ConnectedFd);
            }                                   
        }
    }
}




