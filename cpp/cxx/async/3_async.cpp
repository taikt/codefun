// https://thispointer.com//c11-multithreading-part-9-stdasync-tutorial-example/

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <future>
using namespace std::chrono;
std::string fetchDataFromDB(std::string recvdData)
{
    // Make sure that function takes 5 seconds to complete
    std::this_thread::sleep_for(seconds(5));
    //Do stuff like creating DB Connection and fetching Data
    return "DB_" + recvdData;
}
std::string fetchDataFromFile(std::string recvdData)
{
    // Make sure that function takes 5 seconds to complete
    std::this_thread::sleep_for(seconds(5));
    //Do stuff like fetching Data File
    return "File_" + recvdData;
}
int main()
{
    // Get Start Time
    system_clock::time_point start = system_clock::now();

/*
    First argument in std::async is launch policy, it control the asynchronous behaviour of std::async.
    We can create std::async with 3 different launch policies i.e.

    std::launch::async
        It guarantees the asynchronous behaviour i.e. passed function will be executed in seperate thread.
    std::launch::deferred
        Non asynchronous behaviour i.e. Function will be called when other thread will call get() on future to access the shared state.
    std::launch::async | std::launch::deferred
        Its the default behaviour. With this launch policy it can run asynchronously or not depending on the load on system.
        But we have no control over it
*/
    // Calling std::async with function pointer as callback
#if 0
    std::future<std::string> resultFromDB = std::async(std::launch::async, fetchDataFromDB, "Data");
#endif

    //Calling std::async with lambda function
#if 1
    std::future<std::string> resultFromDB = std::async([](std::string recvdData){
                        std::this_thread::sleep_for (seconds(5));
                        //Do stuff like creating DB Connection and fetching Data
                        return "DB_" + recvdData;
                    }, "Data");
#endif
    //Fetch Data from File
    std::string fileData = fetchDataFromFile("Data");
    //Fetch Data from DB
    // Will block till data is available in future<std::string> object.
    std::string dbData = resultFromDB.get();
    // Get End Time
    auto end = system_clock::now();
    auto diff = duration_cast < std::chrono::seconds > (end - start).count();
    std::cout << "Total Time Taken = " << diff << " Seconds" << std::endl;
    //Combine The Data
    std::string data = dbData + " :: " + fileData;
    //Printing the combined Data
    std::cout << "Data = " << data << std::endl;
    return 0;
}

