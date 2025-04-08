#include "EventNotifier.h"
#ifndef IEVENTHANDLER_H_
#define IEVENTHANDLER_H_

enum EventType
{
	ACCEPT_SECURED_EVENT = 1,
	ACCEPT_UNSECURED_EVENT = 2,
	READ_SECURED_EVENT = 3,
	READ_UNSECURED_EVENT = 4
};

class EventNotifier;
class IEventHandler
{
public:
	//Handle event
	virtual void handleEvent(int sd,EventType et) = 0;
	virtual void setReactor(EventNotifier* reactor)=0;
};
#endif /* IEVENTHANDLER_H_ */
