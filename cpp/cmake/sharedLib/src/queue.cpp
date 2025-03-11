#include "queue.h"

Queue::Queue() 
{ 
    mHead = NULL;
}


Queue::~Queue() 
{
    NODE *tmp;
    while(mHead) {
	tmp = mHead;
	mHead = mHead->next;
	delete tmp;
    }
}


void Queue::push(int e)
{
    NODE *ptr = new node;
    ptr->data = e;
    ptr->next = NULL;
    if(mHead == NULL) {
	mHead = ptr;
	return;
    }
    NODE *cur = mHead;
    while(cur) {
	if(cur->next == NULL) {
	    cur->next = ptr;
	    return;
	}
	cur = cur->next;
    }
}


int Queue::pop()
{
    if(mHead == NULL) return NULL;
    NODE *tmp = mHead;
    int d = mHead->data;
    mHead = mHead->next;
    delete tmp;
    return d;
}