#include <iostream>


class Queue
{
public:
    Queue();
    ~Queue();
    void push(int e);
    int pop();
    
private:
    struct node
    {
        int data;
        node* next;
    };
    typedef node NODE;
    NODE* mHead;
};