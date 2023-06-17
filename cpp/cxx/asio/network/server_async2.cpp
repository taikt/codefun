//importing libraries
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>

class con_handler : public std::enable_shared_from_this<con_handler>
{
private:
    boost::asio::ip::tcp::socket sock;
    std::string msg;
    enum { max_length = 10 };
    char data[max_length];

public:
    con_handler(boost::asio::io_service& io_service): sock(io_service){}
    //socket creation
    boost::asio::ip::tcp::socket& socket()
    {
        return sock;
    }

    void read()
    {
        sock.async_read_some(
        boost::asio::buffer(data, max_length),
        boost::bind(&con_handler::handle_read,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }

    void write()
    {
        msg = data;
        sock.async_write_some(
        boost::asio::buffer(msg, max_length),
        boost::bind(&con_handler::handle_write,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
    {
        if (!err) {
            std::cout << data << std::endl;
        } else {
            std::cerr << "error: " << err.message() << std::endl;
            sock.close();
        }
        write();
    }
    void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
    {
    if (err)
    {
    std::cerr << "error: " << err.message() << std::endl;
    sock.close();
    }
    }
};

class Server
{
private:
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::io_service &io_service;
    void start_accept()
    {
        // socket
        std::shared_ptr<con_handler> con = std::make_shared<con_handler>(io_service);

        // asynchronous accept operation and wait for a new connection.
        acceptor_.async_accept(con->socket(),
        boost::bind(&Server::handle_accept, this, con,
        boost::asio::placeholders::error));
    }
public:
    //constructor for accepting connection from client
    Server(boost::asio::io_service& io_service, std::string address, unsigned int port):
    acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address), port)),io_service(io_service)
    {
        start_accept();
    }
    void handle_accept(std::shared_ptr<con_handler> con, const boost::system::error_code& err)
    {
        if (!err) {
            con->read();
        }
        start_accept();
    }
};


int main(int argc, char *argv[])
{

    if(argc != 3)
    {
        std::cout<<"Wrong parameter\n"<<"Example usage ./async_server 127.0.0.1 1234"<<std::endl;
        return -1;
    }

    std::string address = argv[1];
    unsigned int port = std::atoi(argv[2]);

    boost::asio::io_service io_service;
    Server server(io_service,address,port);
    io_service.run();

    return 0;
}
