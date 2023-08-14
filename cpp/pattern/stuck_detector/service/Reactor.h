#ifndef REQUESTDISPATCHER_H_
#define REQUESTDISPATCHER_H_

#include "EventNotifier.h"

class Reactor : public EventNotifier
{
public:
	Reactor();
	static Reactor* getInstance();
	void initialize();
	void deinitialize();
    void listenSocket();
    void addClient(int sockfd);
    void removeClient(int sockfd);
	void startRunning();

private:
    void handleEvents() override;
    void createServerSocket();
    int serverSockfd_;
    //set of socket descriptors
    fd_set readfds_;

private:
    static const int MAX_CONN_LISTEN = 200;
    static const int MAX_CONNECTED_CLIENTS = 300;
    int client_socket[MAX_CONNECTED_CLIENTS];

	static Reactor* singleton_;
};


#endif /* REQUESTDISPATCHER_H_ */
