#include "connectionManager.h"
#include "Log.h"
#define LOG_TAG "ConnectionManager"

Session::Session(boost::asio::io_service& io_service):sock(io_service) {}
 stream_protocol::socket& Session::socket() {
    return sock;
}

void Session::read() {
    sock.async_read_some(
        boost::asio::buffer(data,MAX_LENGTH),
        boost::bind(&Session::handleRead, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred)
        );
}
void Session::handleRead(const boost::system::error_code& err, size_t bytes_transfered) {
    if (!err) {
        LOGI("read data from client: %s",data);
    } else {
        LOGI("error to read data");
        sock.close();
    }
}


Server::Server(boost::asio::io_service& io_service, const std::string& file)
    : acceptor_(io_service, stream_protocol::endpoint(file)), io_service(io_service)
{
    startAccept();
}


void Server::startAccept() {
    LOGI("startAccept");
    std::shared_ptr<Session> sec = std::make_shared<Session>(io_service);

    acceptor_.async_accept(sec->socket(),
        boost::bind(&Server::handleAccept, this, sec,
        boost::asio::placeholders::error)
        );
}

void Server::handleAccept(std::shared_ptr<Session> sec, const boost::system::error_code& err) {
    LOGI("handleAccept");
    if (!err) {
        sec->read();
    }
    startAccept();
}





