#ifndef MESSAGEUNSECUREDREADERHANDLER_H_
#define MESSAGEUNSECUREDREADERHANDLER_H_

#include "IEventHandler.h"
#include "Dispatcher.h"
class Reactor;
class StatusWriter;

class UnsecReaderHandler:public IEventHandler
{
public:
   static UnsecReaderHandler* getInstance();
   virtual void handleEvent(int sd,EventType) override;
   virtual void setReactor(EventNotifier* reactor) override;
   void parse_socket_data(int fd);
   void set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher);

private:
	UnsecReaderHandler();
	UnsecReaderHandler(UnsecReaderHandler const&);
   void operator=(UnsecReaderHandler const&);
   Reactor * reactor_;
   static UnsecReaderHandler* singleton_;
   std::shared_ptr<Dispatcher> taskDispatcher_ = nullptr;
};

#endif /* MESSAGEUNSECUREDREADERHANDLER_H_ */
