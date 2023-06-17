#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <thread>

struct Service {
    ~Service() {
        std::cout << "clean and stop\n";
        io_service.post([this]() {
            work.reset(); // let io_service run out of work
        });

        if (worker.joinable())
            worker.join();
    }

    void start() {
        assert(!worker.joinable());
        worker = std::thread([this] {
		        //for (int i = 0; i< 10; i++)
			//std::cout <<i<<"\n";	
			int i = 0;
			while (1) {
			    boost::this_thread::sleep( boost::posix_time::milliseconds( 1000 ) );			    
			    std::cout <<i++<<"\n";
		    	    //if (i == 15) break;	    
			    
			}
			io_service.run(); 
		        std::cout << "exiting thread\n";
			});
    }

private:
    boost::asio::io_service io_service;
    std::thread worker;
    boost::optional<boost::asio::io_service::work> work{io_service};
};

int main() {
    {
        Service test;
        test.start();

        //std::cin.ignore(1024, '\n');
        std::cout << "Start shutdown\n";
    }

    std::cout << "exiting program\n"; // never printed
}
