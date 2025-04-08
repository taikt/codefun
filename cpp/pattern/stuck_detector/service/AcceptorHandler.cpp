#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <strings.h>
#include "basic_types.h"
#include "Reactor.h"
#include "AcceptorHandler.h"
#include "ReaderHandler.h"
#include "SocketException.h"
#include "CommData.h"

//#include <dlt/dlt.h>
//DLT_IMPORT_CONTEXT(stuckdetectorctx);
#include "Log.h"
#define LOG_TAG "AcceptorHandler"


using namespace std;

AcceptorHandler* AcceptorHandler::singleton_= nullptr;

/*
AcceptorHandler::AcceptorHandler(int sockFileDescriptor,Reactor *reqDisp)
:serverSockfd_(sockFileDescriptor),reactor_(reqDisp)
{
	//reqDisp->registerEventHandler(this);
}
*/

static AcceptorHandler* AcceptorHandler::getInstance()
{
   if (singleton_ == nullptr) {
		singleton_ = new AcceptorHandler();
   }
   return singleton_;
}

void AcceptorHandler::setReactor(EventNotifier* reactor) {
	reactor_ = reactor;
	LOGI("register event handler to Reactor");

}

void AcceptorHandler::handleEvent(int sd,EventType et)
{
	int new_sockfd,addrlen;
	//struct sockaddr_in address;
	struct sockaddr address;
	addrlen = sizeof(address);

	if (et == ACCEPT_EVENT)
	{
		cout<<"Accept event occured"<<endl;

        LOGI("Accept event occured");
		//Accept new connection

	    if ((new_sockfd = accept(sd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
        {
            cout<<"Failed to accept new connection"<<endl;
            //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Failed to accept new connection"));
             LOGI("Failed to accept new connection");
            //throw SocketException("Could not accept connection");
        }

        OperationType welcome = WELCOME_MESSAGE;
        //send welcome message to client
        if( send(new_sockfd, (char *)&welcome ,sizeof(OperationType), 0) != sizeof(OperationType) )
        {
            cout<<"Could not send";
            //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Could not send"));
             LOGI("Could not send");
            close(new_sockfd);
        }
        cout<<"Welcome message sent successfully"<<endl;
         LOGI("Welcome message sent successfully");
        //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Welcome message sent successfully"));
		if (reactor_ != nullptr) {
			LOGI("add new client, sockfd=%d",new_sockfd);
        	reactor_->addClient(new_sockfd);
		}
        //ReaderHandler::getInstance().setReactor(reactor_);

        //reactor_->setEventHandler(&ReaderHandler::getInstance());
	}
	else
	{
		//Should not be here
		cout<<"Wrong event type called"<<endl;
	}

}


