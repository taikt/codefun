#pragma once
#include "EventNotifier.h"
#include <winsock2.h>
#include <windows.h>
#include "NetworkTCP.h"
#include <set>

#define SECURE_PORT 2222
#define UNSECURE_PORT 2223

using namespace std;

class Reactor : public EventNotifier
{
public:
	Reactor();
	static Reactor* getInstance();
	void initialize();
    void deinitialize();
    void addClient(int sockfd);
    void removeClient(int sockfd);
	void startRunning();

private:
    void handleEvents() override;
    void createServerSocket();
    FD_SET readfds_;
    static Reactor* singleton_;

public:
    static const int MAX_CONN_LISTEN = 200;
    static const int MAX_CONNECTED_CLIENTS = 300;
    int client_socket[MAX_CONNECTED_CLIENTS];
    TTcpListenPort* secTcpListenPort;
    TTcpListenPort* unsecTcpListenPort;
    TTcpConnectedPort* unsecTcpConnectedPort;
    set<int> sec_set;
    set<int> unsec_set;
};

