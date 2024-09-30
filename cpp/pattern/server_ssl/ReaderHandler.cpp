#include <string>
#include <fstream>
#include "Reactor.h"
#include "AcceptorHandler.h"
#include "ReaderHandler.h"
#include <unordered_map>
#include <openssl/ssl.h>
#include "UserAuth.h"
#include "VehicleFinder.h"
#include "NetworkTCP.h"
using namespace std;


#define BUFFER_SIZE 2048

extern std::unordered_map<uint32_t, SSL*> ssl_map;
extern std::unordered_map<string, uint32_t> user_sessionid_map;
extern std::unordered_map<uint32_t, string> sessionid_user_map;
extern std::unordered_map<string, uint32_t> query_track;

ReaderHandler* ReaderHandler::singleton_= nullptr;

ReaderHandler::ReaderHandler()
{
}

ReaderHandler* ReaderHandler::getInstance()
{

    if (singleton_ == nullptr) {
        singleton_ = new ReaderHandler();
    }
    return singleton_;
}

void ReaderHandler::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void ReaderHandler::handleEvent(int sd,EventType et)
{
	if (et == READ_SECURED_EVENT) {
		if (ssl_map.count(sd)) {
			// printf("read data for client socket=%d\n", sd);
			parse_socket_data(sd);			
		}
		else {
			printf("cannot find mapped ssl with client yet:%d\n", sd);
		}
	}
}

void ReaderHandler::setReactor(EventNotifier* reactor) {
	reactor_ = static_cast<Reactor*>(reactor);
}

void ReaderHandler::parse_socket_data(int fd)
{
    unsigned char data[BUFFER_SIZE];
    int len;

    if (taskDispatcher_ == nullptr) {
        printf("task dispatcher is null\n");
        return;
    }

    // TOTO: convert ntoh for data
    if (!ssl_map.count(fd)) {
        printf("cannot find ssl map for client:%d, client is disconnected\n", fd);
        return;
    }

    if (!ReadDataSSL(ssl_map[fd], data)) {
        printf("SSL read on socket failed, client:%d disconnect\n", fd);
        Reactor::getInstance()->removeClient(fd);
        ssl_map.erase(fd);
        // remove session ID associated with this client (fd)
        // when user logout, also need to remove session ID
        // shoule remove session id if client suddently disconnected because network error?
        // TODO: free ssl resource associtated with this fd            
        CLOSE_SOCKET(fd);
        return;
    }

    RequestMsg* request_msg = new RequestMsg();
    deserialize_request(request_msg, data);
    string payload(request_msg->payload);

    if (request_msg->msg_type == LOGIN_REQ) {
        printf("receive login request: %d, id_len=%d, pw_len=%d, payload=%s, fd=%d\n", fd, request_msg->id_len, request_msg->pw_len, request_msg->payload, fd);
        int id_len = request_msg->id_len;
        printf("id len=%d\n", id_len);        
        // pass directly request_msg->id_len cause error!!! => never pass pointer to dispatcher
        taskDispatcher_->deliverTaskIn([=] {userAuth::getInstance()->handle_login_request(payload, id_len, fd); }); 

    }
    else if (request_msg->msg_type == PLATE_QUERY) {
#ifdef OLD_PLATE_QUERY        
        printf("receive plate query for client: %d, plate:%s, sessionId=%d\n", fd, request_msg->payload, request_msg->session_id);
        // should verify session id in DB? which can slow performance due to query delay
        if (!sessionid_user_map.count(request_msg->session_id) || (sessionid_user_map.count(request_msg->session_id) && user_sessionid_map[sessionid_user_map[request_msg->session_id]] != request_msg->session_id)) {
            printf("session ID is not found: %d\n", request_msg->session_id);
            string data_resp;
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, data_resp, false); });
        }
        else {                    
            query_track[sessionid_user_map[request_msg->session_id]]++;

            taskDispatcher_->deliverTaskIn([=] {vehicleFinder::getInstance()->handle_plate_query(payload, fd); });
        }
#else  
        printf("<--receive plate query for client: %d, plate_num:%d, sessionId=%d\n", fd, request_msg->id_len, request_msg->session_id);
        //printf("payload=%s\n", payload.c_str());
        
        uint8_t num_plate = request_msg->id_len;
        char* payload_char;

	   // should verify session id in DB? which can slow performance due to query delay
        if (!sessionid_user_map.count(request_msg->session_id) || (sessionid_user_map.count(request_msg->session_id) && user_sessionid_map[sessionid_user_map[request_msg->session_id]] != request_msg->session_id)) {
            printf("session ID is not found: %d\n", request_msg->session_id);           
            payload_char = (char*)calloc(4, sizeof(char));
            // need to send query_id
            memcpy(payload_char, request_msg->payload, 4); 
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, 0, payload_char, 4); });
        }
        else {                    
            query_track[sessionid_user_map[request_msg->session_id]]++;
            uint32_t sessionId = request_msg->session_id;
            payload_char = (char*)calloc(request_msg->payload_len, sizeof(char));
            //for PLATE_QUERY message, id_len used for num_plate
			memcpy(payload_char, request_msg->payload, request_msg->payload_len);
            taskDispatcher_->deliverTaskIn([=] {vehicleFinder::getInstance()->handle_plate_query(num_plate, payload_char, fd, sessionId); });
        }      
#endif        
    }
    else if (request_msg->msg_type == PING_REQ) {
        //printf("receive ping request from client:%d\n",fd);
        // for ping request, server immediately send ping response without sending to dispatcher?
        send_ping_resp(fd, request_msg->session_id);
    }
    else if (request_msg->msg_type == LOGOUT_REQ) {
        printf("receive logout request from client:%d\n", fd);
        uint32_t sessionId = request_msg->session_id;
        taskDispatcher_->deliverTaskIn([=] {userAuth::getInstance()->handle_logout_request(sessionId,fd); });
    }
    else {
        printf("unknown request type:%d\n", request_msg->msg_type);
    }

    delete request_msg;
    request_msg = nullptr;
}

void ReaderHandler::send_ping_resp(int fd, uint32_t sessionid) {
    if (!ssl_map.count(fd)) {
        printf("cannot find ssl map for client:%d,  client is disconnected\n", fd);
        return;
    }

    ResponseMsg* response_msg = new ResponseMsg();
    response_msg->protocol_version = PROTOCOL_VERSION;
    response_msg->msg_type = PING_RESP;
    response_msg->return_code = SUCCESS;
    response_msg->session_id = sessionid;

    unsigned char data[BUFFER_SIZE];
    serialize_response(response_msg, data);

    if (WriteDataSSL(ssl_map[fd], data, sizeof(data))) {
        //printf("write ping response to client:%d\n",fd);
    }
    else {
        printf("Cannot send ping response to client: %d\n",fd);
        Reactor::getInstance()->removeClient(fd);
        ssl_map.erase(fd);
        CLOSE_SOCKET(fd);
    }

    delete response_msg;
    response_msg = nullptr;
}



