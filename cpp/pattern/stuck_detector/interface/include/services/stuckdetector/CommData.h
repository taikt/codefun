#include <stdint.h>
#include <string>



#ifndef COMMDATA_H
#define COMMDATA_H

#ifdef __cplusplus
extern "C" {
#endif

# pragma pack (1)	// Defined to avoid aligning in memory allocation

#define STATUS_UPDATE_LENGTH 690

typedef enum type
{
    STATUS_OK,
    STATUS_ERROR,
    WELCOME_MESSAGE
} OperationType;
/*
typedef struct packet
{
	//First 4 byte can be status if came from server or client's ID
	union
	{
      OperationType	type;		///< Type of operation
	  int           clientId;
	};
    char statusUpdate[STATUS_UPDATE_LENGTH];
} CommData;
*/
/*
typedef struct packet
{
	union
	{
      OperationType	type;		///< Type of operation
	  int           clientId;
	};
	uint8_t timeout;
	std::string filename;
	std::string funcname;
	uint32_t line;
} CommData;
*/
/*
class CommData {
	public:
		CommData() {filename="";funcname="";}
		OperationType	type;
		int  clientId;
		uint8_t timeout;
		std::string filename;
		std::string funcname;
		uint32_t line;
};
*/
class CommData {
	public:
		//CommData() {filename="";funcname="";}
		CommData() {}
		OperationType	type;
		int  clientId;
		int timeout;
		char filename[300];
		char funcname[100];
		int tid;
		int pgid;//change to pid_t
		uint32_t line;
};





__attribute__((always_inline)) inline void initializeCommData( CommData * pData )
{
	bzero( pData, sizeof( CommData ) ) ; // legacy => change to memset()
}

__attribute__((always_inline)) inline bool validateCommData( const CommData * pData )
{
	bool result = false;
	if( sizeof(*pData)  == sizeof(CommData ) )
	{
		result = true;
	}
    return result;
}


# pragma pack ()

#ifdef __cplusplus
}
#endif


#endif	//COMMDATA_H

