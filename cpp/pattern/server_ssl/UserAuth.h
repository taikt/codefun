#pragma once
#include <iostream>
#include "Dispatcher.h"
#include "authDb.h"
using namespace std;

class userAuth {
public:
	bool validate_password(string userId, string password);
	void send_authenticate_response(int fd, bool valid_user, int session_id);
	void handle_login_request(string payload, int id_len, int fd);
	static userAuth* getInstance();
	void set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher);
	void set_db_manager(std::shared_ptr<AuthDb> authen_db);
	void load_mobile_id();
	bool validate_biometric(string userId);
	void handle_logout_request(uint32_t sessionId, uint32_t fd);
	void send_logout_response(uint32_t fd);

private:
	static userAuth* singleton_;
	std::shared_ptr<Dispatcher> taskDispatcher_ = nullptr;
	std::shared_ptr<AuthDb> _authen_db;
};