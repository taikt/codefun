#pragma once
#include <iostream>
#include <db.h> 
using namespace std;

//#define OLD_PLATE_QUERY
#define PARTIAL_MATCH

#define BUFFER_SIZE 2048

#define PARTIAL_THRESHOLD "partial_threshold"

class VehicleDb {
public:
	VehicleDb(const char* db_file);
	bool create_db();
#ifdef OLD_PLATE_QUERY  	
	bool retrieve_vehicle_info(string _key, char* vehicle_info);
#else
	uint8_t retrieve_vehicle_info(char* _key, u_int32_t _key_len, char** vehicle_info , uint8_t  max_partial_cnt, uint32_t sessionId);
#endif

private:
	DB* dbp;
	const char* _db_file;
	int findBestmatch(char* first, u_int32_t first_len, char* second);
};