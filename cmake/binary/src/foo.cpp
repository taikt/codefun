#include "queue.h"
#include "foo.h"
#include <iostream>
#include <string>

int main()
{
    std::cout << "cnt:"<< MAX_CNT <<"\n";
    Queue *i = new Queue();
    i->push(10);
    i->push(20);
    std::cout<<"value: "<<i->pop()<<"\n";
    std::cout<<"value: "<<i->pop()<<"\n";
    
    delete i;

    return 0;
}