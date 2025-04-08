// https://thispointer.com/c11-multithreading-part-10-packaged_task-example-and-tutorial/
// https://stackoverflow.com/questions/18143661/what-is-the-difference-between-packaged-task-and-async

#include <thread>
#include <thread>
#include <future>
#include <string>
#include <bits/stdc++.h>
#include <time.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>


using namespace std;

string getData(string token) {
    int run=0;
    while (1) {
        this_thread::sleep_for(chrono::milliseconds(500));
        cout<<"run="<<run<<"\n";
        run++;
        if (run == 5) {
            string data = "mydata" + token;
            return data;
        }
    }
}

int main() {
#if 1
    // use function pointer
    packaged_task<string(string)> task(getData);
    // A packaged_task won't start on it's own, you have to invoke it, or move it to another thread to invoke inside thread
    // On the other hand, std::async with launch::async will try to run the task in a different thread
    // check detailed here:
    // https://stackoverflow.com/questions/18143661/what-is-the-difference-between-packaged-task-and-async


    future<string> result = task.get_future();
    cout<<"I am waiting\n";
    thread th(move(task), " + pending data");

    // blocked here until thread running getData finish
    th.join();
    cout<<"ok:"<<result.get()<<"\n";
#endif // 0

#if 0
    // use lambda
    packaged_task<string(string)> task([](string token) {
        int run=0;
        while (1) {
            this_thread::sleep_for(chrono::milliseconds(200));
            cout<<"run="<<run<<"\n";
            run++;
            if (run == 5) {
                string data = "mydata" + token;
                return data;
            }
        }
    }
    );

    future<string> result = task.get_future();
    cout<<"I am waiting\n";
    thread th(move(task), "pending data");

    th.join();
    cout<<"ok:"<<result.get()<<"\n";
#endif // 1
    return 0;
}
