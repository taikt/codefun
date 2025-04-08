#include "EventNotifier.h"

#ifndef IEVENTHANDLER_H_
#define IEVENTHANDLER_H_

enum EventType
{
  ACCEPT_EVENT = 1,
  READ_EVENT = 2
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
