#include <memory>
#include <string>
#include "Promise.h"
//#pragma once

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

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
#endif