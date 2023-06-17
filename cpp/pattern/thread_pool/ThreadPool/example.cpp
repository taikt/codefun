#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"
using namespace std;

int main()
{

    ThreadPool pool(8);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
#if 0
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
#endif // 0
                std::cout << "hello "<<i<<"\n";
                std::this_thread::sleep_for(std::chrono::seconds(3));
                //std::cout << "world " << i <<"\n";

                return i*i;
            })
        );
    }

    cout<<"return result:\n";
for(auto && result: results)
//        std::cout << result.get() << ' ';
//    std::cout << std::endl;

     std::cout <<"*"<<result.get()<<",";

    return 0;
}
