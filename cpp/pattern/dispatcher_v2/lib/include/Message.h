/*
* tai2.tran
*/

#include <memory>
#include <string>
#include "Promise.h"

#ifndef MESSAGE_HPP
#define MESSAGE_HPP


namespace kt {

template<typename vValue>
class Promise;


class Message 
{
public:
    

public:
    Message(int id_):
    id(id_){}
    //Message(Message& other);
    //virtual ~Message();

    int id;
    std::string payload;

    
    std::shared_ptr<Promise<int>> m_promise;

 
};

}

#endif