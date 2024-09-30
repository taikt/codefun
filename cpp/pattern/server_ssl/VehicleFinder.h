#include <iostream>
#include "Dispatcher.h"
#include "VehicleDb.h"
using namespace std;

class vehicleFinder {
public:
#ifdef OLD_PLATE_QUERY 
	void send_plate_response(int fd, string plate_info, bool found);
	void handle_plate_query(string plate, int fd);
#else
	void send_plate_response(int fd, uint8_t num_vehicle, char* plate_info, uint32_t plate_info_len);
	void handle_plate_query(uint8_t num_plate, char* payload, int fd, uint32_t sessionId);
#endif	
	static vehicleFinder* getInstance();
	void set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher);
	void set_db_manager(std::shared_ptr<VehicleDb> vehicle_db);

private:
	static vehicleFinder* singleton_;
	std::shared_ptr<Dispatcher> taskDispatcher_ = nullptr;
	std::shared_ptr<VehicleDb> _vehicle_db;
};
