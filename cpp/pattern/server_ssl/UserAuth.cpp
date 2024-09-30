#include "UserAuth.h"
#include "Reactor.h"
#include <unordered_map>
#include <db.h> 
#include "NetworkTCP.h"
#include <openssl/ssl.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>

#include <atomic>
#include <condition_variable>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#define BUFFER_SIZE 2048

extern std::unordered_map<string, uint32_t> query_track;
extern std::unordered_map<string, uint32_t> user_sessionid_map;
extern std::unordered_map<uint32_t, string> sessionid_user_map;
extern std::unordered_map<uint32_t, SSL*> ssl_map;
extern std::unordered_map<uint32_t, string> mobileid_user_map;

extern std::condition_variable bio_cv;
extern std::mutex bio_cv_m;
extern std::atomic<int> bio_flag;
extern uint8_t biometric_stt;
extern std::unordered_map<string, TTcpConnectedPort*> user_unsecfd;

userAuth* userAuth::singleton_ = nullptr;


userAuth* userAuth::getInstance()
{
    if (singleton_ == nullptr) {
        singleton_ = new userAuth();
    }
    return singleton_;
}

void userAuth::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void userAuth::set_db_manager(std::shared_ptr<AuthDb> authen_db) {
    _authen_db = authen_db;
}

bool userAuth::validate_password(string userId, string password) {
    // TODO: validate userID, password
    DBT key, data;
    char DBRecord[2048];
    int rc;

    printf("[request] userID:%s, password:%s\n", userId.c_str(), password.c_str());

    char out[BUFFER_SIZE];
    if (_authen_db->retrieve_cred(userId, out)) {
        string cred((char*)out);
        string id, pw, mobileId;
        //size_t pos = cred.find('\n');
        //id = cred.substr(0, pos);
        //pw = cred.substr(pos + 1);
        
        istringstream is(cred);
        vector<string> v((istream_iterator<string>(is)),istream_iterator<string>());
        id = v[0];
        pw = v[1];
        mobileId = v[2];
        printf("cre:%s\n",cred.c_str());
        printf("[DB] userID:%s, password:%s, mobileId=%s\n", id.c_str(), pw.c_str(), mobileId.c_str());
        if (pw != password) return false;
        else return true;        
    } else {
        printf("[DB] cannot find user:%s in DB\n", userId.c_str());
        return false;
    }
}

bool userAuth::validate_biometric(string userId) {
    if (user_unsecfd.count(userId)) {
        unsigned char data[BUFFER_SIZE];
        data[0] = PROTOCOL_VERSION;
        data[1] = MOBILE_AUTH_REQ;
        int result, SIZE = 2;

        if ((result = WriteDataTcp(user_unsecfd[userId], (unsigned char*)data, SIZE)) == SIZE)
            printf("sent biometric authen request\n");
        else {
            printf("cannot send biometric authen request for user:%d\n",userId.c_str());
            return false;
        }


        std::unique_lock<std::mutex> lk(bio_cv_m);
        auto now = std::chrono::system_clock::now();
        if (bio_cv.wait_until(lk, now + 20000ms, []() {return bio_flag == 1; })) {
            printf("receiveid biometric response from mobile app\n");
            if (biometric_stt == 1) {
                printf("biometric authentication is successfully\n");
                return true;
            }
            else if (biometric_stt == 0) {
                printf("biometric authentication is fail\n");
                return false;
            }
            else {
                printf("biometric authentication: unkknown status\n");
                return false;
            }
        }
        else {
            printf("timeout (20 sec), no receive biometric response from mobile app\n");
            return false;
        }          
    }
    else {
        printf("cannot find user:%s registered with its mobileid\n",userId.c_str());
        return false;
    }

}

void userAuth::handle_login_request(string payload, int id_len, int fd) {
    if (taskDispatcher_ == nullptr) {
        printf("task dispatcher is null\n");
        return;
    }

    string userId = payload.substr(0, id_len);
    string password = payload.substr(id_len);
    string woBio = "admin";

    printf("userID:%s, password:%s\n", userId.c_str(), password.c_str());
    uint32_t ses_id = 0;

    query_track[userId]++;

    if (validate_password(userId, password)) {

        int res = userId.compare(woBio);

        if ((res ==0) || validate_biometric(userId)) {
            if (user_sessionid_map.count(userId)) {
                // if same client(user) access with different devices, should take a same session ID?
                printf("this user is on activating\n");
                ses_id = user_sessionid_map[userId];
            }
            else {
                ses_id = fd; // chose client socket as session Id
                user_sessionid_map[userId] = ses_id;
                sessionid_user_map[ses_id] = userId;
            }
            taskDispatcher_->deliverTaskOut([=] {userAuth::getInstance()->send_authenticate_response(fd, true, ses_id); });
        }
        else {
            printf("fail to validate biometric\n");
            taskDispatcher_->deliverTaskOut([=] {userAuth::getInstance()->send_authenticate_response(fd, false, ses_id); });
        }
    }
    else {        
        taskDispatcher_->deliverTaskOut([=] {userAuth::getInstance()->send_authenticate_response(fd, false, ses_id); });
    }
}

void userAuth::send_authenticate_response(int fd, bool valid_user, int session_id) {
    ResponseMsg* response_msg = new ResponseMsg();
    response_msg->protocol_version = PROTOCOL_VERSION;
    response_msg->msg_type = LOGIN_RESP;

    if (valid_user) {
        printf("user is valid\n");
        // create random session ID, save to memory to store user session                
        response_msg->return_code = SUCCESS;
        response_msg->session_id = session_id;
        response_msg->payload_len = 0;
    }
    else {
        printf("user is invalid\n");
        response_msg->return_code = FAIL;
        response_msg->session_id = 0;
        response_msg->payload_len = 0;
    }

    unsigned char data[BUFFER_SIZE];
    serialize_response(response_msg, data);

    
    if (WriteDataSSL(ssl_map[fd], data, sizeof(data))) {
        printf("send ssl ok \n");
    }
    else {
        printf("cannot send ssl\n");
    }
    

    delete response_msg;
    response_msg = nullptr;
}

void userAuth::load_mobile_id() {
    _authen_db->load_mobile_id();
}

void userAuth::send_logout_response(uint32_t fd) {
    printf("send logout response for client: %d\n", fd);
    if (ssl_map.count(fd) && ssl_map[fd]) {
        ResponseMsg* response_msg = new ResponseMsg();
        response_msg->protocol_version = PROTOCOL_VERSION;
        response_msg->msg_type = LOGOUT_RESP;
        response_msg->return_code = SUCCESS;

        unsigned char data[BUFFER_SIZE];
        serialize_response(response_msg, data);


        if (WriteDataSSL(ssl_map[fd], data, sizeof(data))) {
            printf("send ssl ok \n");
        }
        else {
            printf("cannot send ssl\n");
        }

        delete response_msg;
        response_msg = nullptr;
    }
}

void userAuth::handle_logout_request(uint32_t sessionId, uint32_t fd) {
    // is that nesseary to send logout response?
    send_logout_response(fd);

    // free resourse
    if (sessionid_user_map.count(sessionId)) {
        string userId = sessionid_user_map[sessionId];
        sessionid_user_map.erase(sessionId);
        if (user_sessionid_map.count(userId)) {
            user_sessionid_map.erase(userId);
        }
    }
    else {
        printf("handling logout: cannot find sessionId:%d in stored map\n",sessionId);
    }

    if (ssl_map.count(fd) && ssl_map[fd]) {
        SSL_shutdown(ssl_map[fd]);
        SSL_free(ssl_map[fd]);
    }

    // remove client
    Reactor::getInstance()->removeClient(fd);
}