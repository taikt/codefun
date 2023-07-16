#include <memory>
#include <string>
#pragma once


class Message 
{
public:
    Message(int id_, std::string str_):
    id(id_), str(str_){}
    //Message(Message& other);
    //virtual ~Message();

    int id;
    std::string str;
};
