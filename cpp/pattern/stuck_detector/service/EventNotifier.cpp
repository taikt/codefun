#include "EventNotifier.h"
#include "IEventHandler.h"

#include "Log.h"
#define LOG_TAG "EventNotifier"

/**
 * Constructor
 */
EventNotifier::EventNotifier()
{
}

/**
 * Destructor
 */
EventNotifier::~EventNotifier()
{
}

/**
 * Method sets handler object
 *
 */
void EventNotifier::registerEventHandler(EventType type, IEventHandler* handler)
{
	if (handlers_.count(type)) {
		LOGI("handler for event:%d is existed, skip adding",type);
	} else {
		LOGI("add handler for event:%d",type);
		handlers_[type] = handler;
	}
}

void EventNotifier::removeEventHandler(EventType type) {
	std::map<int,IEventHandler*>::iterator it;
	if (handlers_.count(type)) {
		LOGI("handler for event:%d is existed, delete it",type);
		it = handlers_.find (type);
		handlers_.erase(it);
	} else {
		LOGI("cannot find handler for event:%d",type);
	}
}

/**
 * Methods notifies registered handler
 */
void EventNotifier::dispatchEvent(int sd,EventType et)
{
	if (handlers_.count(et)) {
		//LOGI("dispatch event");
		handlers_[et]->handleEvent(sd,et);
	} else {
		LOGI("cannot find handler for event");
	}
}




