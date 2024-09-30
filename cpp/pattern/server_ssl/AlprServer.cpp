// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string.h>
#include "NetworkTCP.h"
#include <Windows.h>
#include <db.h> 
#include <openssl/ssl.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <unordered_map>
#include "DeadlineTimer.h"
#include <iostream>
#include <chrono>
#include <ctime> 
#include <fstream>
#include <iomanip>
#include <sstream>
#include "AcceptorHandler.h"
#include "ReaderHandler.h"
#include "UnsecAcceptorHandler.h"
#include "UnsecReaderHandler.h"
#include "userAuth.h"
#include "vehicleFinder.h"
#include "vehicleDb.h"
#include "authDb.h"
#include "Reactor.h"
using namespace std;

#define KEY_FILE_LEN 32
#define BUFFER_SIZE 2048
#define CA_CERT_FILE "ca_cert.pem"
#define SERVER_CERT_FILE "server_cert.pem"
#define SERVER_PRIVATE_KEY_FILE "server_key.pem"

#define IN_THREAD_CNT "in_thread_cnt"
#define OUT_THREAD_CNT "out_thread_cnt"


#ifdef PARTIAL_MATCH
#define VEHICLE_DB "licenseplate_btree_25m_dup.db"
#else
#define VEHICLE_DB "licenseplate.db"
#endif
#define AUTH_DB "userid.db"

std::unordered_map<uint32_t, SSL*> ssl_map;
std::unordered_map<string, uint32_t> user_sessionid_map;
std::unordered_map<uint32_t, string> sessionid_user_map;
std::unordered_map<string, uint32_t> query_track;
std::unordered_map<string, uint32_t> configured_param;
std::unordered_map<uint32_t, string> mobileid_user_map;
std::unordered_map<string, TTcpConnectedPort*> user_unsecfd;
std::unordered_map<string, uint32_t> partial_match;
std::unordered_map<string, uint32_t> no_partial_match;

auto start = std::chrono::system_clock::now();

std::condition_variable bio_cv;
std::mutex bio_cv_m;
std::atomic<int> bio_flag{ 0 };
uint8_t biometric_stt = 0;


SSL_CTX* AlprServer::get_server_context(const char* ca_pem,
    const char* cert_pem,
    const char* key_pem) {
    SSL_CTX* ctx;

    /* Get a default context */
    if (!(ctx = SSL_CTX_new(SSLv23_server_method()))) {
        fprintf(stderr, "SSL_CTX_new failed\n");
        return NULL;
    }

    /* Set the CA file location for the server */
    if (SSL_CTX_load_verify_locations(ctx, ca_pem, NULL) != 1) {
        fprintf(stderr, "Could not set the CA file location\n");
        goto fail;
    }

    /* Load the client's CA file location as well */
    SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(ca_pem));

    /* Set the server's certificate signed by the CA */
    if (SSL_CTX_use_certificate_file(ctx, cert_pem, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Could not set the server's certificate\n");
        goto fail;
    }

    /* Set the server's key for the above certificate */
    if (SSL_CTX_use_PrivateKey_file(ctx, key_pem, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Could not set the server's key\n");
        goto fail;
    }

    /* We've loaded both certificate and the key, check if they match */
    if (SSL_CTX_check_private_key(ctx) != 1) {
        fprintf(stderr, "Server's certificate and the key don't match\n");
        goto fail;
    }

    /* We won't handle incomplete read/writes due to renegotiation */
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    /* Specify that we need to verify the client as well */
    SSL_CTX_set_verify(ctx,
        SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
        NULL);

    /* We accept only certificates signed only by the CA himself */
    SSL_CTX_set_verify_depth(ctx, 1);

    /* Done, return the context */
    return ctx;

fail:
    SSL_CTX_free(ctx);
    return NULL;
}

void AlprServer::track_average_query() {
    //TODO: when user logout, need remove query_track for that user.
    std::ofstream outfile;
    outfile.open("server_tracking\\average_query.txt", std::ios_base::app); 
    
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    unordered_map<string, uint32_t>::iterator it;
    string userId;
    uint32_t total_query = 0;
    
    for (it = query_track.begin(); it != query_track.end(); it++) {
        total_query += it->second;
    }
    for (it = query_track.begin(); it != query_track.end(); it++) {        
        char str[26];
        time_t result = time(NULL);
        ctime_s(str, sizeof str, &result);
        string currentTime(str);
        string _currentTime = currentTime.substr(0, currentTime.size()-1); // remove last characer '\n'
        
        outfile << setw(30) << left << _currentTime.c_str() << setw(12) << left << it->first << setw(25) << left << (float)(it->second / elapsed_seconds.count())<<
            setw(30) << left << (float)(total_query/ elapsed_seconds.count())<<"\n";
    }
    
    outfile.close();
}

void AlprServer::track_partial_match() {
    std::ofstream outfile;
    outfile.open("server_tracking\\partial_match.txt", std::ios_base::app);

 
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    unordered_map<string, uint32_t>::iterator it;
    string userId;
    uint32_t total_match = 0;

    for (it = partial_match.begin(); it != partial_match.end(); it++) {
        total_match += it->second;
    }


    for (it = partial_match.begin(); it != partial_match.end(); it++) {
        char str[26];
        time_t result = time(NULL);
        ctime_s(str, sizeof str, &result);
        string currentTime(str);
        string _currentTime = currentTime.substr(0, currentTime.size() - 1); // remove last characer '\n'

        outfile << setw(30) << left << _currentTime.c_str() << setw(12) << left << it->first << setw(25) << left << it->second << setw(30) << left << total_match << "\n";
    }

    outfile.close();
}

void AlprServer::track_no_partial_match() {
    std::ofstream outfile;
    outfile.open("server_tracking\\no_match.txt", std::ios_base::app);
  
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    unordered_map<string, uint32_t>::iterator it;
    string userId;
    uint32_t total_no_match = 0;

    for (it = no_partial_match.begin(); it != no_partial_match.end(); it++) {
        total_no_match += it->second;
    }


    for (it = no_partial_match.begin(); it != no_partial_match.end(); it++) {
        char str[26];
        time_t result = time(NULL);
        ctime_s(str, sizeof str, &result);
        string currentTime(str);
        string _currentTime = currentTime.substr(0, currentTime.size() - 1); // remove last characer '\n'

        outfile << setw(30) << left << _currentTime.c_str() << setw(12) << left << it->first << setw(25) << left << it->second << setw(30) << left << total_no_match << "\n";
    }

    outfile.close();
}

bool AlprServer::init_ssl() {
    char ca_pem[KEY_FILE_LEN] = CA_CERT_FILE;
    char cert_pem[KEY_FILE_LEN] = SERVER_CERT_FILE;
    char key_pem[KEY_FILE_LEN] = SERVER_PRIVATE_KEY_FILE;

    /* Initialize OpenSSL */
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    /* Get a server context for our use */
    if (!(svc_ctx = get_server_context(ca_pem, cert_pem, key_pem))) {
        return -1;
    }

    AcceptorHandler::getInstance()->set_context(svc_ctx);
}

void AlprServer::load_config() {
    printf("load config file\n");
    std::ifstream file("vehicle_param.cfg");
    std::string line;
    
    while (std::getline(file, line))
    {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value)) {
                printf("key:%s, value:%s\n", key.c_str(), value.c_str());
                configured_param[key.c_str()] = stoi(value);
            }
        }
    }
}

void AlprServer::initiate() {
    load_config();
    init_ssl();

    Reactor::getInstance()->initialize();
    Reactor::getInstance()->registerEventHandler(ACCEPT_SECURED_EVENT, AcceptorHandler::getInstance());
    Reactor::getInstance()->registerEventHandler(ACCEPT_UNSECURED_EVENT, UnsecAcceptorHandler::getInstance());
    Reactor::getInstance()->registerEventHandler(READ_SECURED_EVENT, ReaderHandler::getInstance());
    Reactor::getInstance()->registerEventHandler(READ_UNSECURED_EVENT, UnsecReaderHandler::getInstance());

    AcceptorHandler::getInstance()->setReactor(Reactor::getInstance());
    UnsecAcceptorHandler::getInstance()->setReactor(Reactor::getInstance());

    ReaderHandler::getInstance()->setReactor(Reactor::getInstance());
    UnsecReaderHandler::getInstance()->setReactor(Reactor::getInstance());

    taskDispatcher_ = std::shared_ptr<Dispatcher>(new Dispatcher(configured_param[IN_THREAD_CNT], configured_param[OUT_THREAD_CNT]));
    vehicle_db_ = std::shared_ptr<VehicleDb>(new VehicleDb(VEHICLE_DB));
    vehicle_db_->create_db();

    authen_db_ = std::shared_ptr<AuthDb>(new AuthDb(AUTH_DB));
    authen_db_->create_db();

    AcceptorHandler::getInstance()->set_dispatcher(taskDispatcher_);
    UnsecAcceptorHandler::getInstance()->set_dispatcher(taskDispatcher_);
    ReaderHandler::getInstance()->set_dispatcher(taskDispatcher_);
    UnsecReaderHandler::getInstance()->set_dispatcher(taskDispatcher_);
    userAuth::getInstance()->set_dispatcher(taskDispatcher_);
    vehicleFinder::getInstance()->set_dispatcher(taskDispatcher_);

    userAuth::getInstance()->set_db_manager(authen_db_);
    vehicleFinder::getInstance()->set_db_manager(vehicle_db_);
    userAuth::getInstance()->load_mobile_id();
}


