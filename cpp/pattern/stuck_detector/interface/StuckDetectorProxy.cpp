#include "services/stuckdetector/StuckDetectorProxy.h"
#include "Log.h"
#include <cstring>

#define LOG_TAG "StuckDetectorProxy"

//#include "ADADiagManagerService.h"

// TODO: close socket when client down

StuckDetectorProxy* StuckDetectorProxy::singleton_= nullptr;


void StuckDetectorProxy::heartBeat(int timeout, char* const file, int line, char* const func) {
    //LOGI("send heartbeat");
	CommData output;
	bzero( &output, sizeof( CommData ) ) ;

    int tid = syscall(SYS_gettid);
    int pid = getpid();
    int pgid = getpgid(pid);
    //LOGI("tid=%d, pid=%d, pgid=%d",tid,pid, pgid);

    output.clientId= 1;
	output.timeout = timeout;
	//output.filename = file;
	output.line = line;
	//output.funcname = func;
    std::strcpy(output.filename,file);
	std::strcpy(output.funcname,func);
    output.tid = tid;
    output.pgid = pgid;



	int n = write(sockfd,&output,sizeof(CommData));
    /*
	if (n < 0)
	  LOGI("ERROR writing to socket");
    else LOGI("heartbeat ok");
    */

}


void StuckDetectorProxy::connectSocket(){
    int n,repeat=3;
    struct sockaddr_un serv_addr;
    struct hostent *server;

    CommData input;

    int clientId;
    //add terminating string char
    char printBuffer[STATUS_UPDATE_LENGTH + 1];
    char buffer[BUFF_SIZE];

    //initialize input data structure
    initializeCommData(&input);

    clientId = 1;




	sockfd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (sockfd < 0)
        LOGI("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sun_family = AF_LOCAL;
	strcpy(serv_addr.sun_path, DEBUGGER_SOCKET_PATH);


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
       LOGI("ERROR connecting");
    else LOGI("connect server success");


    bzero(buffer,BUFF_SIZE);

    n = read(sockfd,&input.type,sizeof(input.type));
    if (n < 0)
         LOGI("ERROR reading from socket");

    if(input.type == WELCOME_MESSAGE)
    {
    	//LOGI("Successfuly received welcome message %d\n",input.type);
    }
    else
    {
    	//LOGI("Welcome message did not arrive %d\n",input.type);
    }


}

