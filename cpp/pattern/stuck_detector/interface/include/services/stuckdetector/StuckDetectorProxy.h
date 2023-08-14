#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> //read(),write(),close()
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//#include "type_def.h"
#include "CommData.h"
//#include "utils.h"
//#include "ap-main.h"
#include <sys/syscall.h>


#define BUFF_SIZE 256
#define SERV_PORT 5000
#define DEBUGGER_SOCKET_PATH "/dev/stuckdetector_socket"
#define TIMEOUT 15

//#include "CommData.h"
#include <string>




class StuckDetectorProxy {
public:
	StuckDetectorProxy(){}
	static StuckDetectorProxy* getInstance() {
		if(singleton_ == nullptr) {
	        singleton_ = new StuckDetectorProxy();
	    }
		return singleton_;
	}
	void heartBeat(int timeout, char* const file, int line, char* const func);
	void connectSocket();

private:
	static StuckDetectorProxy* singleton_;
	int sockfd;

};


class Delegator
{
private:
	char* const _func;
	char* const _file;
	int _line;
public:

	Delegator(char* const file, int line, char* const func)
		:_func(func), _file(file), _line(line)
    {
        StuckDetectorProxy::getInstance()->heartBeat(TIMEOUT,_file,_line,_func);
    }
    ~Delegator()
    {
        StuckDetectorProxy::getInstance()->heartBeat(-1,_file,_line,_func);
    }
};

#define TRACE_START \
    class Delegator __inst_using__(__FILE__,__LINE__,__FUNCTION__);

