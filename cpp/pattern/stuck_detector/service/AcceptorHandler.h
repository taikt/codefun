#ifndef MESSAGEACCEPTORHANDLER_H_
#define MESSAGEACCEPTORHANDLER_H_

#include "IEventHandler.h"


class AcceptorHandler : public IEventHandler
{
public:
	static AcceptorHandler* getInstance();
	virtual void handleEvent(int sd,EventType et);
	virtual void setReactor(EventNotifier* reactor);

private:
	Reactor *reactor_;
	static AcceptorHandler* singleton_;
};



#endif /* MESSAGEACCEPTORHANDLER_H_ */
