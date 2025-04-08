//------------------------------------------------------------------------------------------------
// File: NetworkTCP.h
// Project: LG Exec Ed Program
// Versions:
// 1.0 April 2017 - initial version
// Provides the ability to send and recvive UDP Packets for both Window and linux platforms
//------------------------------------------------------------------------------------------------
#ifndef NetworkTCPH
#define NetworkTCPH
#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <BaseTsd.h>
#include <cstdint>
#include <openssl/ssl.h>
typedef SSIZE_T ssize_t;
#define  CLOSE_SOCKET closesocket
#define  SOCKET_FD_TYPE SOCKET
#define  BAD_SOCKET_FD INVALID_SOCKET
#define PAYLOAD_SIZE 1024
#define PROTOCOL_VERSION 1.0
#define BUFFER_SIZE 2048

//------------------------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------------------------

typedef struct
{
 SOCKET_FD_TYPE ListenFd;
} TTcpListenPort;

typedef struct
{
 SOCKET_FD_TYPE ConnectedFd;
} TTcpConnectedPort;


enum message_type {
    LOGIN_REQ   = 0x00,
    LOGOUT_REQ  = 0x01,
    PLATE_QUERY = 0x02,
    PING_REQ    = 0x03,

    MOBILE_AUTH_REQ = 0x04,
    MOBILE_AUTH_RESP = 0x05,
    MOBILE_INFO_IND = 0x06,

    LOGIN_RESP  = 0x10,
    LOGOUT_RESP = 0x20,
    PLATE_RESP  = 0x30,
    PING_RESP  = 0x40,   
};

enum result_code {
    SUCCESS = 0,
    FAIL
};

typedef struct Request_msg_t
{
    uint8_t protocol_version;
    uint8_t msg_type;
    uint8_t id_len;
    uint8_t pw_len;
    uint32_t session_id;
    uint32_t payload_len;
    char payload[PAYLOAD_SIZE];
} RequestMsg;

typedef struct Response_msg_t
{
    uint8_t protocol_version;
    uint8_t msg_type;
    uint8_t return_code;
    uint8_t reserved; // unused
    uint32_t session_id;
    uint32_t payload_len;
    char payload[PAYLOAD_SIZE];
} ResponseMsg;

//------------------------------------------------------------------------------------------------
//  Function Prototypes 
//------------------------------------------------------------------------------------------------
TTcpListenPort *OpenTcpListenPort(short localport);
void CloseTcpListenPort(TTcpListenPort **TcpListenPort);
TTcpConnectedPort *AcceptTcpConnection(TTcpListenPort *TcpListenPort, 
                       struct sockaddr_in *cli_addr,socklen_t *clilen);
TTcpConnectedPort *OpenTcpConnection(const char *remotehostname, const char * remoteportno);
void CloseTcpConnectedPort(TTcpConnectedPort **TcpConnectedPort);
ssize_t ReadDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length);
ssize_t BytesAvailableTcp(TTcpConnectedPort* TcpConnectedPort);
ssize_t WriteDataTcp(TTcpConnectedPort *TcpConnectedPort,unsigned char *data, size_t length);
void serialize_request(RequestMsg &msg, unsigned char* data);
void deserialize_request(RequestMsg* msg, unsigned char* data);
void serialize_response(ResponseMsg* msg, unsigned char* data);
void deserialize_response(ResponseMsg* msg, unsigned char* data);
bool WriteDataSSL(SSL* ssl, unsigned char* data, size_t length);
bool ReadDataSSL(SSL* ssl, unsigned char* data);
#endif
//------------------------------------------------------------------------------------------------
//END of Include
//------------------------------------------------------------------------------------------------




