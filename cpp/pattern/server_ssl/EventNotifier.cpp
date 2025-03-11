#include "EventNotifier.h"
#include "IEventHandler.h"

EventNotifier::EventNotifier(){
}

EventNotifier::~EventNotifier(){
}

void EventNotifier::registerEventHandler(EventType type, IEventHandler* handler)
{
	if (handlers_.count(type)) {
		//printf("handler for event:%d is existed, skip adding",type);
	} else {
		//printf("add handler for event:%d",type);
		handlers_[type] = handler;
	}
}

void EventNotifier::removeEventHandler(EventType type) {
	std::map<int,IEventHandler*>::iterator it;
	if (handlers_.count(type)) {
		//printf("handler for event:%d is existed, delete it",type);
		it = handlers_.find (type);
		handlers_.erase(it);
	} else {
		//printf("cannot find handler for event:%d",type);
	}
}

/**
 * Methods notifies registered handler
 */
void EventNotifier::dispatchEvent(int sd,EventType et)
{
	if (handlers_.count(et)) {
		//printf("dispatch event\n");
		handlers_[et]->handleEvent(sd,et);
	} else {
		//printf("cannot find handler for event\n");
	}
}




