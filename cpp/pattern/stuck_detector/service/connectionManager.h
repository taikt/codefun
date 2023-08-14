#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>

using boost::asio::local::stream_protocol;

#define MAX_LENGTH 100

class Session: public std::enable_shared_from_this<Session> {
private:
	//boost::asio::ip::tcp::socket sock;
	stream_protocol::socket sock;
	std::string msg;
	char data[MAX_LENGTH];

public:
	Session(boost::asio::io_service& io_service);

	//boost::asio::ip::tcp::socket& socket();
	stream_protocol::socket& socket();
	void read();
	void handleRead(const boost::system::error_code& err, size_t bytes_transfered);

};

class Server {
private:
	//boost::asio::ip::tcp::acceptor acceptor_;
	stream_protocol::acceptor acceptor_;
	boost::asio::io_service& io_service;
	void startAccept();

public:
	Server(boost::asio::io_service& io_service, const std::string& file);
	void handleAccept(std::shared_ptr<Session> sec, const boost::system::error_code& err);

};

