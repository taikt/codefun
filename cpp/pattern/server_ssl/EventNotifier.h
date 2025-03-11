#ifndef EVENTNOTIFIER_H_
#define EVENTNOTIFIER_H_
#include "IEventHandler.h"
#include <map>

class EventNotifier
{
public:
	EventNotifier();
	virtual ~EventNotifier();
	void registerEventHandler(EventType type, IEventHandler* handler);
	void removeEventHandler(EventType type);
	virtual void handleEvents()=0;
	virtual void dispatchEvent(int sd,EventType);
private:
	std::map<int,IEventHandler*> handlers_;
};
#endif /* EVENTNOTIFIER_H_ */
