#ifndef MESSAGEREADERHANDLER_H_
#define MESSAGEREADERHANDLER_H_

#include "IEventHandler.h"
#include "Dispatcher.h"
class Reactor;
class StatusWriter;

class ReaderHandler:public IEventHandler
{
public:
   static ReaderHandler* getInstance();
   virtual void handleEvent(int sd,EventType) override;
   virtual void setReactor(EventNotifier* reactor) override;
   void parse_socket_data(int fd);
   void set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher);
   void send_ping_resp(int fd, uint32_t sessionid);

private:
   ReaderHandler();
   ReaderHandler(ReaderHandler const&);
   void operator=(ReaderHandler const&);
   Reactor * reactor_;
   static ReaderHandler* singleton_;
   std::shared_ptr<Dispatcher> taskDispatcher_ = nullptr;
};

#endif /* MESSAGEREADERHANDLER_H_ */
