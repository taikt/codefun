#ifndef MESSAGEREADERHANDLER_H_
#define MESSAGEREADERHANDLER_H_

#include "IEventHandler.h"
class Reactor;
class StatusWriter;

class ReaderHandler:public IEventHandler
{
public:
   static ReaderHandler* getInstance();
   virtual void handleEvent(int sd,EventType) override;
   virtual void setReactor(EventNotifier* reactor) override;

private:
   ReaderHandler();
   ReaderHandler(ReaderHandler const&);
   void operator=(ReaderHandler const&);
   Reactor * reactor_;
   static ReaderHandler* singleton_;
};



#endif /* MESSAGEREADERHANDLER_H_ */
