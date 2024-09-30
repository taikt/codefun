#include "Reactor.h"
#include "AcceptorHandler.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

Reactor* Reactor::singleton_= nullptr;

Reactor::Reactor():
    secTcpListenPort(nullptr),
    unsecTcpListenPort(nullptr)
{
}

Reactor* Reactor::getInstance()
{
   if (singleton_ == nullptr) {
		singleton_ = new Reactor();
   }
   return singleton_;
}

void Reactor::startRunning()
{
   while (1)
   {
       handleEvents();
   }
}


void Reactor::handleEvents() 
{
	int max_sd,sd;

    //clear the socket set
    FD_ZERO(&readfds_);

    //add master socket to set    
    FD_SET(secTcpListenPort->ListenFd, &readfds_);
    FD_SET(unsecTcpListenPort->ListenFd, &readfds_);

    max_sd = max(secTcpListenPort->ListenFd, unsecTcpListenPort->ListenFd);

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
   
    //If something happened on the master socket , then its an incoming connection
    if (FD_ISSET(secTcpListenPort->ListenFd, &readfds_))
    {
        //printf("accept ssl event occur\n");
        this->dispatchEvent(secTcpListenPort->ListenFd, ACCEPT_SECURED_EVENT);
    }
    if (FD_ISSET(unsecTcpListenPort->ListenFd, &readfds_))
    {
        //printf("accept non-ssl event occur\n");
        this->dispatchEvent(unsecTcpListenPort->ListenFd, ACCEPT_UNSECURED_EVENT);
    }
    else
    {
    	//Data arrived on one of clients
         for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
         {
             sd = client_socket[i];
             if (FD_ISSET( sd , &readfds_))
             {
                 if (this->sec_set.count(sd)) {
                     this->dispatchEvent(sd, READ_SECURED_EVENT);
                 }
                 else if (this->unsec_set.count(sd)) {
                     printf("dispatch read event for unsecured client:%d\n", sd);
                     this->dispatchEvent(sd, READ_UNSECURED_EVENT);
                 }
                 else {
                     printf("client:%d is not binding to secured/unsecured set\n", sd);
                 }
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
    FD_SET(secTcpListenPort->ListenFd, &readfds_);
    FD_SET(unsecTcpListenPort->ListenFd, &readfds_);

}

void Reactor::deinitialize()
{
    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CONNECTED_CLIENTS; i++)
    {
       if( client_socket[i] != 0 )
       {    	   
           CLOSE_SOCKET(client_socket[i]);
    	   client_socket[i] = 0;
       }
    }

    if (secTcpListenPort->ListenFd != 0)
    {
        CLOSE_SOCKET(secTcpListenPort->ListenFd);
    }

    if (unsecTcpListenPort->ListenFd != 0)
    {
        CLOSE_SOCKET(unsecTcpListenPort->ListenFd);
    }
}

void Reactor::createServerSocket()
{
   
    if ((unsecTcpListenPort = OpenTcpListenPort(UNSECURE_PORT)) == NULL)
    {
        std::cout << "[Unsecured connect] OpenTcpListenPortFailed\n";
        return;
    }
    else {
        printf("success setup unsecured(non-ssl) server socket:%d\n", (int)unsecTcpListenPort->ListenFd);
    }

    if ((secTcpListenPort = OpenTcpListenPort(SECURE_PORT)) == NULL)
    {
        std::cout << "[secured connect] OpenTcpListenPortFailed\n";
        return;
    }
    else {
        printf("success setup secured(ssl) server socket:%d\n", (int)secTcpListenPort->ListenFd);
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

    if (sec_set.count(sockfd)) {
        sec_set.erase(sockfd);
    }
    CLOSE_SOCKET(sockfd);
}

