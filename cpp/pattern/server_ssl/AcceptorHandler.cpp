#include "Reactor.h"
#include "AcceptorHandler.h"
#include "ReaderHandler.h"
#include <openssl/ssl.h>
#include <unordered_map>
using namespace std;

extern std::unordered_map<uint32_t, SSL*> ssl_map;

AcceptorHandler* AcceptorHandler::singleton_= nullptr;
extern std::shared_ptr<Dispatcher> taskDispatcher_;

AcceptorHandler* AcceptorHandler::getInstance()
{
   if (singleton_ == nullptr) {
		singleton_ = new AcceptorHandler();
   }
   return singleton_;
}

void AcceptorHandler::setReactor(EventNotifier* reactor) {
	reactor_ = static_cast<Reactor*>(reactor); // TODO check
}

void AcceptorHandler::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void AcceptorHandler::handleEvent(int sd,EventType et)
{
    struct sockaddr_in cli_addr;
    socklen_t          clilen;
    TTcpConnectedPort* TcpConnectedPort;

    if (taskDispatcher_ == nullptr) {
        printf("task dispatcher is null\n");
        return;
    }

    if (et == ACCEPT_SECURED_EVENT) {
        memset(&cli_addr, '0', sizeof(cli_addr));
        clilen = sizeof(cli_addr);
        if ((TcpConnectedPort = AcceptTcpConnection(reactor_->secTcpListenPort, &cli_addr, &clilen)) == NULL)
        {
            printf("AcceptTcpConnection Failed\n");
            return;
        }
        else {
            printf("handle connection for a client: fd=%d\n", (int)TcpConnectedPort->ConnectedFd);
            if (reactor_ != nullptr) {                
                reactor_->addClient(TcpConnectedPort->ConnectedFd);
                reactor_->sec_set.insert(TcpConnectedPort->ConnectedFd);
            }                       
            taskDispatcher_->deliverTaskIn([=] {handle_client_connection(TcpConnectedPort->ConnectedFd); });
        }

        delete TcpConnectedPort;
        TcpConnectedPort = nullptr;
    }
}

void AcceptorHandler::handle_client_connection(SOCKET_FD_TYPE client_fd) {
    int rc;
    unsigned short PlateStringLength;
    char PlateString[1024] = { 0 };


    //DB* dbp; /* DB structure handle */
    uint32_t flags; /* database open flags */
    int ret; /* function return value */
    ssize_t result;
    SSL* ssl;

    /* Get an SSL handle from the context */
    if (ctx_ != nullptr) {
        if (!(ssl = SSL_new(ctx_))) {
            printf("Could not get an SSL handle from the context\n");
            return;
        }

        /* Associate the newly accepted connection with this handle */
        SSL_set_fd(ssl, client_fd);

        /* Now perform handshake */
        if ((rc = SSL_accept(ssl)) != 1) {
            printf("Could not perform SSL handshake\n");
            if (rc != 0) {
                SSL_shutdown(ssl);
            }
            SSL_free(ssl);
        }
     
        //inet_ntop(AF_INET, &(cli_addr.sin_addr), cliAddr, INET_ADDRSTRLEN);
        // printf("SSL handshake successful with client: %s\n", cliAddr);
        printf("SSL handshake successful with client\n");
        ssl_map[client_fd] = ssl;
    }
}

void AcceptorHandler::set_context(SSL_CTX* ctx) {
    ctx_ = ctx;
}


