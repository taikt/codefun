#include <iostream>
#include "vehicleFinder.h"
#include <unordered_map>

#include <db.h> 
#include "NetworkTCP.h"
#include <openssl/ssl.h>
#include "Reactor.h"
//#define BUFFER_SIZE 1024
#define BUFFER_SIZE 2048

CRITICAL_SECTION cs_send;

extern std::unordered_map<uint32_t, SSL*> ssl_map;
extern std::unordered_map<uint32_t, string> sessionid_user_map;
extern std::unordered_map<string, uint32_t> no_partial_match;

vehicleFinder* vehicleFinder::singleton_ = nullptr;


vehicleFinder* vehicleFinder::getInstance()
{
    if (singleton_ == nullptr) {
        singleton_ = new vehicleFinder();
        InitializeCriticalSection(&cs_send);
    }
    return singleton_;
}

void vehicleFinder::set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher) {
    taskDispatcher_ = _dispatcher;
}

void vehicleFinder::set_db_manager(std::shared_ptr<VehicleDb> vehicle_db) {
    _vehicle_db = vehicle_db;
}

#ifdef OLD_PLATE_QUERY 
void vehicleFinder::handle_plate_query(string plate, int fd) {
    DBT key, data;
    char DBRecord[2048];

    printf("handle_plate_query: plate query:%s for client:%d\n", plate.c_str(), fd);
    char out[BUFFER_SIZE];

    //EnterCriticalSection(&cs);
    bool result = _vehicle_db->retrieve_vehicle_info(plate, out);
    //LeaveCriticalSection(&cs);
    if (result) {
        //printf("plate infor ->%s\n", (char*)out);
        string data_resp((char*)out);
        if (taskDispatcher_ != nullptr)
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, data_resp, true); });
        else printf("taskDispatcher_ is null");
    }
    else {
        //printf("not found:%d", fd);
        string data_resp;
        if (taskDispatcher_ != nullptr)
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, data_resp, false); });
        else printf("taskDispatcher_ is null");
    }
}

void vehicleFinder::send_plate_response(int fd, string plate_info, bool found) {
    EnterCriticalSection(&cs_send);
    printf("send_plate_response\n");
    if (!ssl_map.count(fd)) {
        printf("cannot find ssl map for client:%d,  client is disconnected\n", fd);
        return;
    }

    int rc;
    ResponseMsg* response_msg = new ResponseMsg();
    response_msg->protocol_version = PROTOCOL_VERSION;
    response_msg->msg_type = PLATE_RESP;

    if (found) {
        //printf("found plate in DB\n");
        // create random session ID, save to memory to store user session                
        response_msg->return_code = SUCCESS;
        response_msg->session_id = 1256;// TODO: need get session ID of this user
        response_msg->payload_len = plate_info.size();
        strncpy_s(response_msg->payload, plate_info.c_str(), plate_info.size());
    }
    else {
        //printf("not found plate in DB\n");
        response_msg->return_code = FAIL;
        response_msg->session_id = 0;
        response_msg->payload_len = 0;
    }

    unsigned char data[BUFFER_SIZE];
    serialize_response(response_msg, data);

    if (WriteDataSSL(ssl_map[fd], data, sizeof(data))) {
        printf("write plate response ok\n");
    }
    else {
        printf("Cannot send plate response to client\n");
        Reactor::getInstance()->removeClient(fd);
        ssl_map.erase(fd);
        CLOSE_SOCKET(fd);
    }

    delete response_msg;
    response_msg = nullptr;
    LeaveCriticalSection(&cs_send);
}
#else

void vehicleFinder::handle_plate_query(uint8_t num_plate, char* payload, int fd, uint32_t sessionId) {
    uint32_t cnt;
    uint32_t plate_len = 0;
    uint32_t vehicle_info_len = 0;
    uint8_t  num_vehicle = 0;
    uint8_t max_partial_cnt = 4;
    
    printf("handle_plate_query\n");
    char* p = (char*)payload;
    char out[BUFFER_SIZE] = { 0, };
    char* p_out = out;

    //printf("payload: %s\n", p+8);

    max_partial_cnt = (num_plate >= 2)? 2: 4;

    // copy query_id to out buffer
    memcpy(p_out, p, 4);    
    p += 4;
    p_out += 4;


#if 1
    for(cnt = 0; cnt < num_plate; cnt++)
    {
        memcpy(&plate_len, p, 4);
        p += 4;

        //printf("plate_len %d\n", plate_len);
        num_vehicle += _vehicle_db->retrieve_vehicle_info(p, plate_len, &p_out, max_partial_cnt, sessionId);

        p += plate_len;
    }
#else // test    
    string test;
    num_plate = 2;
    test = "ABC0067";
    for (cnt = 0; cnt < num_plate; cnt++)
    {
        plate_len = test.size();        

        printf("plate_len %d\n", plate_len);
        num_vehicle += _vehicle_db->retrieve_vehicle_info((char*)test.c_str(), plate_len, &p_out, max_partial_cnt);

        p += plate_len;
        test = "ABC006";
    }
#endif

    char* resp_payload;

    if (num_vehicle > 0) {
        uint32_t payload_len = p_out - out;

        if (taskDispatcher_ != nullptr)
        {
            resp_payload = (char*)calloc((int)(payload_len), sizeof(char));
            memcpy(resp_payload, out, payload_len);
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, num_vehicle, resp_payload, payload_len); });
        }
        else printf("taskDispatcher_ is null");

    }
    else {
        if (taskDispatcher_ != nullptr)
        {
            resp_payload = (char*)calloc(4, sizeof(char));
            memcpy(resp_payload, out, 4);
            taskDispatcher_->deliverTaskOut([=] {vehicleFinder::getInstance()->send_plate_response(fd, num_vehicle, resp_payload, 4); });
        }
        else printf("taskDispatcher_ is null");

        // track no match
        if (sessionid_user_map.count(sessionId)) {
            no_partial_match[sessionid_user_map[sessionId]]++;
        }
    }

    free(payload);
}

void vehicleFinder::send_plate_response(int fd, uint8_t num_vehicle, char* plate_info, uint32_t plate_info_len) {
    //EnterCriticalSection(&cs_send);
    printf("-->send_plate_response, num_vehicle:%d, vehicle_info_len:%d\n", num_vehicle, plate_info_len);
    if (!ssl_map.count(fd)) {
        printf("cannot find ssl map for client:%d,  client is disconnected\n", fd);
        return;
    }

    int rc;
    ResponseMsg* response_msg = new ResponseMsg();
    response_msg->protocol_version = PROTOCOL_VERSION;
    response_msg->msg_type = PLATE_RESP;
    response_msg->reserved = num_vehicle;

    if (num_vehicle) {
        //printf("found plate in DB\n");
        // create random session ID, save to memory to store user session                
        response_msg->return_code = SUCCESS;
        response_msg->session_id = 1256;// TODO: need get session ID of this user
        response_msg->payload_len = plate_info_len;
        memcpy(response_msg->payload, plate_info, plate_info_len);
        printf("first info:%s\n", response_msg->payload+8);
    }
    else {
        //printf("not found plate in DB\n");
        response_msg->return_code = FAIL;
        response_msg->session_id = 1256;// TODO: need get session ID of this user
        response_msg->payload_len = 0;

        response_msg->payload_len = plate_info_len;
        memcpy(response_msg->payload, plate_info, plate_info_len);
        printf("No payload\n");
    }

    unsigned char data[BUFFER_SIZE];
    serialize_response(response_msg, data);

    if (WriteDataSSL(ssl_map[fd], data, sizeof(data))) {
        printf("write plate response ok\n");
    }
    else {
        printf("Cannot send plate response to client\n");
        Reactor::getInstance()->removeClient(fd);
        ssl_map.erase(fd);
        CLOSE_SOCKET(fd);
    }

    free(plate_info);

    delete response_msg;
    response_msg = nullptr;
//    LeaveCriticalSection(&cs_send);
}
#endif