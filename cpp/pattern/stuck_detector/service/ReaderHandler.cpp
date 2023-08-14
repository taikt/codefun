#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <strings.h>
#include <string>
#include <fstream>
#include <map>
#include "basic_types.h"
#include "Reactor.h"
#include "AcceptorHandler.h"
#include "ReaderHandler.h"
#include "StatusWriter.h"
#include "CommData.h"
#include "StuckDetectorManager.h"


#include "Log.h"
#define LOG_TAG "ReaderHandler"


using namespace std;

ReaderHandler* ReaderHandler::singleton_= nullptr;

ReaderHandler::ReaderHandler()
{
}

static ReaderHandler* ReaderHandler::getInstance()
{
    /*
    static ReaderHandler  instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
    return instance;
    */
    if (singleton_ == nullptr) {
        singleton_ = new ReaderHandler();
    }
    return singleton_;
}


void ReaderHandler::handleEvent(int sd,EventType et)
{
	// If this is accept event switch handler in dispatcher to AcceptorHandler
	if(et == ACCEPT_EVENT)
	{
		cout<<"Accept event in Reader"<<endl;
	}

	if (et == READ_EVENT)
	{
		CommData input;
		CommData output;

        //LOGI("Read event in ReaderHandler");
        //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Read event in ReaderHandler"));
		struct sockaddr_in address;
		int valread,addrlen;

		//initializeCommData(&input);
		//initializeCommData(&output);


        //Check if it was for closing , and also read the incoming message
        if ((valread = read( sd , &input, sizeof(CommData))) == 0)
        {
           //Somebody disconnected , get his details and print
           getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);

           cout<<"Host disconnected , ip "<< inet_ntoa(address.sin_addr)<<
           	  ", port "<<ntohs(address.sin_port)<<endl;

           //remove socket descriptor from socket descriptor array
           reactor_->removeClient(sd);

           //Close the socket and mark as 0 in list for reuse
           close( sd );

           //remove client stream
           //StatusWriter::getInstance().removeFileStream(6666);
        }
        else
        {
			if(validateCommData(&input))
            //if (1)
			{

                //LOGI("Successfully validated message from client1");
                //LOGI("id=%d, tid=%d, pgid=%d",input.clientId,input.tid,input.pgid);


                //LOGI("file=%s,line:%d,func=%s,timeout=%d",input.filename,input.line,input.funcname,input.timeout);
                std::string _filename(input.filename);
                std::string _funcname(input.funcname);
                StuckDetectorManager::getInstance()->heartBeat(input.timeout,_filename,input.line,_funcname,input.tid,input.pgid,sd);

				//Send acknowledge messgae
		        output.type = STATUS_OK;
		        //send welcome message to client
		        /*
		        if( send(sd, (char *)&output.type ,sizeof(OperationType), 0) != sizeof(OperationType) )
		        {
		            //cout<<"Could not send STATUS_OK"<<endl;
		        }

                //LOGI("STATUS_OK sent successfully");
                */
			}
			else
			{
				cout<<"Validation messge failed"<<endl;
                //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Validation messge failed"));
                LOGI("Validation messge failed");
				//Send not acknowledge messgae
		        output.type = STATUS_ERROR;
		        //send welcome message to client
		        if( send(sd, (char *)&output.type ,sizeof(OperationType), 0) != sizeof(OperationType) )
		        {
		            cout<<"Could not send STATUS_ERROR"<<endl;
		        }
		        cout<<"STATUS_ERROR sent successfully"<<endl;
			}

			cout<<"Logged something from socket: "<<sd<<endl;
        }
	}

}

void ReaderHandler::setReactor(EventNotifier* reactor) {
	reactor_ = reactor;
	LOGI("register event handler to Reactor");
	//reactor_->registerEventHandler(READ_EVENT,this);

	LOGI("set requestDispatcher to StuckDetectorManager");
    StuckDetectorManager::getInstance()->setReactor(reactor_);
}



