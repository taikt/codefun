#pragma once
#include <iostream>
#include <db.h> 
using namespace std;

#define BUFFER_SIZE 2048

class AuthDb {
public:
	bool create_db();
	AuthDb(const char* db_file);
	bool retrieve_cred(string _key, char* cred);
	void load_mobile_id();

private:
	DB* dbp = nullptr;
	const char* _db_file;
};