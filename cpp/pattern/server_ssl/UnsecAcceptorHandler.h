#include "IEventHandler.h"
#include "AlprServer.h"
#include "Dispatcher.h"
#include "Reactor.h"

class UnsecAcceptorHandler : public IEventHandler
{
public:
	static UnsecAcceptorHandler* getInstance();
	virtual void handleEvent(int sd,EventType et);
	virtual void setReactor(EventNotifier* reactor);
	void handle_client_connection(SOCKET_FD_TYPE client_fd);
	void set_context(SSL_CTX* ctx);
	void set_dispatcher(std::shared_ptr<Dispatcher> _dispatcher);

private:
	Reactor* reactor_ = nullptr;
	static UnsecAcceptorHandler* singleton_;
	SSL_CTX* ctx_ = nullptr;
	std::shared_ptr<Dispatcher> taskDispatcher_ = nullptr;
};
