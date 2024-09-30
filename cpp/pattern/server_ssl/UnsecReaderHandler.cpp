#include <string>
#include <fstream>
#include "Reactor.h"
#include "AcceptorHandler.h"
#include "UnsecReaderHandler.h"
#include <unordered_map>
#include <openssl/ssl.h>
#include "UserAuth.h"
#include "VehicleFinder.h"
#include "NetworkTCP.h"
using namespace std;


#define BUFFER_SIZE 2048

extern unordered_map<string, uint32_t> user_sessionid_map;
extern unordered_map<uint32_t, string> sessionid_user_map;
extern unordered_map<string, uint32_t> query_track;
extern unordered_map<uint32_t, string> mobileid_user_map;


extern condition_variable bio_cv;
extern std::atomic<int> bio_flag;
extern uint8_t biometric_stt;

extern std::unordered_map<string, TTcpConnectedPort*> user_unsecfd;


UnsecReaderHandler* UnsecReaderHandler::singleton_= nullptr;

UnsecReaderHandler::UnsecReaderHandler()
{
}

UnsecReaderHandler* UnsecReaderHandler::getInstance()
{

    if (singleton_ == nullptr) {
        singleton_ = new UnsecReaderHandler();
    }
    return singleton_;
}

void UnsecReaderHandler::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void UnsecReaderHandler::handleEvent(int sd,EventType et)
{
	if (et == READ_UNSECURED_EVENT) {		
		printf("read data for unsecured client socket=%d\n", sd);
		parse_socket_data(sd);			
	}
	else {
		//printf("cannot find mapped ssl with client yet:%d\n", sd);
	}	
}

void UnsecReaderHandler::setReactor(EventNotifier* reactor) {
	reactor_ = static_cast<Reactor*>(reactor);
}

void UnsecReaderHandler::parse_socket_data(int fd)
{
    unsigned char data[1024];
    //printf("size=%d\n",(int) sizeof(data));
    ssize_t BytesOnSocket = 0;
    BytesOnSocket = BytesAvailableTcp(reactor_->unsecTcpConnectedPort);

    if (BytesOnSocket == 0) {
        reactor_->removeClient(reactor_->unsecTcpConnectedPort->ConnectedFd);
        reactor_->unsec_set.erase(reactor_->unsecTcpConnectedPort->ConnectedFd);
        CLOSE_SOCKET(reactor_->unsecTcpConnectedPort->ConnectedFd);
        return;
    }
    if (ReadDataTcp(reactor_->unsecTcpConnectedPort, (unsigned char*)&data, BytesOnSocket) != BytesOnSocket)
    {
        printf("ReadDataTcp  error\n");
        reactor_->removeClient(reactor_->unsecTcpConnectedPort->ConnectedFd);
        reactor_->unsec_set.erase(reactor_->unsecTcpConnectedPort->ConnectedFd);
        CLOSE_SOCKET(reactor_->unsecTcpConnectedPort->ConnectedFd);
        return;

    }
    else {
        printf("read data from unsecured socket\n");
    }

    // TODO: process non-ssl data here
    printf("[bio authen] byte[0]=%d, byte[1]=%d, byte[2]=%d\n",data[0],data[1],data[2]);
    if (data[1] == MOBILE_INFO_IND) {
        printf("mobile app id registration\n");

        uint32_t mobileID = data[2];
        if (mobileid_user_map.count(mobileID)) {
            printf("found mobileId=%d in DB\n",mobileID);
            printf("map user:%s with socket fd:%d\n", mobileid_user_map[mobileID].c_str(), (int)reactor_->unsecTcpConnectedPort->ConnectedFd);
            user_unsecfd[mobileid_user_map[mobileID]] = reactor_->unsecTcpConnectedPort;
        }
        else {
            printf("cannot find mobileId:%d in DB\n", mobileID);
            return;
        }
       
    }
    else if (data[1] == MOBILE_AUTH_RESP) {
        printf("biometric authentication response: %d\n",data[2]);
        biometric_stt = data[2];
        bio_flag = 1;
        bio_cv.notify_all();
    }
}



