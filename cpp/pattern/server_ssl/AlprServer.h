#pragma once
#include <memory>
#include "Dispatcher.h"
#include "vehicleDb.h"
#include "authDb.h"

class AlprServer {
public:
	void load_config();
	static void track_average_query(void);
	static void track_partial_match(void);
	static void track_no_partial_match(void);
	SSL_CTX* get_server_context(const char* ca_pem,const char* cert_pem,const char* key_pem);
	bool init_ssl();
	void initiate();

private:
	SSL_CTX* svc_ctx = nullptr;
	std::shared_ptr<Dispatcher> taskDispatcher_;
	std::shared_ptr<VehicleDb> vehicle_db_;
	std::shared_ptr<AuthDb> authen_db_;
};