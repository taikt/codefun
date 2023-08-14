
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <strings.h>
#include "basic_types.h"
#include "Reactor.h"
#include "SocketException.h"
#include "AcceptorHandler.h"
#include <cstring>
#include "Log.h"
//#include "StuckDetectorManager.h"

#define LOG_TAG "Reactor"

#define DEBUGGER_SOCKET_PATH "/dev/stuckdetector_socket"



using namespace std;

Reactor* Reactor::singleton_= nullptr;

Reactor::Reactor():serverSockfd_(0)
{
	//StuckDetectorManager::getInstance()->onInit();
}

static Reactor* Reactor::getInstance()
{
   if (singleton_ == nullptr) {
		singleton_ = new Reactor();
   }
   return singleton_;
}

void Reactor::startRunning()
{

   cout<<"Enter Reactor::dispatcherRoutine"<<endl;
   //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Enter Reactor::dispatcherRoutine"));
   LOGI("Enter Reactor::dispatcherRoutine");


   cout<<"Start listening on created socket"<<endl;
   try
   {
       listenSocket();
       cout<<"Waiting for connection.."<<endl;
       while(1)
       {
    	   handleEvents();
       }
   }
   catch(SocketException &e1)
   {
		cout<<"Exception occurred: "<< e1.what()<<endl;
		deinitialize();
   }

}

// TODO: create EventDemuxlplexer class
void Reactor::handleEvents()
{
    //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("handleEvents"));
    //LOGI("handleEvents");

	int max_sd,sd;

    //clear the socket set
    FD_ZERO(&readfds_);

    //add master socket to set
    FD_SET(this->serverSockfd_, &readfds_);

    max_sd = this->serverSockfd_;

    //add child sockets to set
    for (int i = 0 ; i < MAX_CONNECTED_CLIENTS ; i++)
    {
        //socket descriptor
        sd = client_socket[i];
        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET( sd , &readfds_);
        //highest file descriptor number, need it for the select function
        if(sd > max_sd)
            max_sd = sd;
    }

    //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
    int activity = select( max_sd + 1 , &readfds_ , NULL , NULL , NULL);
    if( (activity<0)  && (errno!=EINTR) )
    {
    	throw SocketException("Error on select");
    }

    //If something happened on the master socket , then its an incoming connection
    if (FD_ISSET(this->serverSockfd_, &readfds_))
    {

		this->dispatchEvent(this->serverSockfd_,ACCEPT_EVENT);
    }
    else
    {
    	//Data arrived on one of clients
         for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
         {
             sd = client_socket[i];
             if (FD_ISSET( sd , &readfds_))
             {
                //LOGI("send read event for client socket:%d,index=%d",sd,i);
            	 this->dispatchEvent(sd,READ_EVENT);
             }
         }
    }
}

void Reactor::initialize()
{
    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
    {
       client_socket[i] = 0;
    }

    //clear the socket set
    FD_ZERO(&readfds_);

    createServerSocket();

    //add master socket to set
    FD_SET(this->serverSockfd_, &readfds_);

}

void Reactor::deinitialize()
{
    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
    {
       if( client_socket[i] != 0 )
       {
    	   close(client_socket[i]);
    	   client_socket[i] = 0;
       }
    }

    if(serverSockfd_!=0)
    {
      close(serverSockfd_);
    }
}

void Reactor::createServerSocket()
{
	int sockfd,res;
	struct sockaddr_un serv_addr;
	int opt = TRUE;
    unlink(DEBUGGER_SOCKET_PATH);

	//sockfd = socket(AF_INET, SOCK_STREAM, 0);
	sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Could not create socket"));
        LOGI("Could not create socket");
    	//throw SocketException("Could not create socket");

    } else LOGI("server create socket ok"); //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("server create socket ok"));

    serverSockfd_ = sockfd;

    bzero((char *) &serv_addr, sizeof(serv_addr));

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("setsockopt failed"));
        LOGI("setsockopt failed");
    	//throw SocketException("setsockopt failed");
    }
/*
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
*/
	serv_addr.sun_family = AF_LOCAL;
    std::strcpy(serv_addr.sun_path, DEBUGGER_SOCKET_PATH);

    res=bind(sockfd, (struct sockaddr *) &serv_addr,
                 sizeof(serv_addr));
    if ( res < 0)
    {
        stringstream s;
       s<<"Could not bind socket error: "<<res;
       //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("Could not bind socket"));
       LOGI("Could not bind socket");
       //throw SocketException(s.str());
    } else LOGI("bind socket ok"); //DLT_LOG(stuckdetectorctx, DLT_LOG_INFO, DLT_STRING("bind socket ok"));

    //cout<<"Socket created and bound on port: "<<ntohs(serv_addr.sin_port)<<endl;

}

void Reactor::listenSocket()
{
	//Listen on bound socket, for max 5 connections
    if (listen(this->serverSockfd_, MAX_CONN_LISTEN) == -1)
    {
       throw SocketException("Could not listen for connection on defined port");
    }
}

void Reactor::addClient(int sockfd)
{
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
    {
        //if position is empty
        if( client_socket[i] == 0 )
        {
            client_socket[i] = sockfd;
            break;
        }
    }
}

void Reactor::removeClient(int sockfd)
{
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
    {
        //if position is empty
        if( client_socket[i] == sockfd )
        {
            client_socket[i] = 0;
            break;
        }
    }
}

