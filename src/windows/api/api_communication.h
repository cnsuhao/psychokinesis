#ifndef _API_COMMUNICATION_H_
#define _API_COMMUNICATION_H_

#include <gloox/client.h>
#include <gloox/loghandler.h>
#include <gloox/messagehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagesessionhandler.h>
#include <boost/thread/thread.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "api.h"

namespace psychokinesis {

class adapter_communication;

class api_communication : public api, public gloox::MessageSessionHandler, public gloox::ConnectionListener, public gloox::MessageHandler, public gloox::LogHandler {
public:
	api_communication()
		: client(NULL), client_thread(NULL), reconnect_semaphore(0), 
		  is_open(false), port(-1), reconnect_timeout(10)
	{}
	virtual bool open();
	
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args);
	
	virtual void close();
	
	friend class adapter_communication;
	
private:
	virtual void handleMessage(const gloox::Message& msg, gloox::MessageSession* session = 0);
	virtual void onConnect();
	virtual void onDisconnect(gloox::ConnectionError e);
	virtual bool onTLSConnect(const gloox::CertInfo& info);
	virtual void handleMessageSession(gloox::MessageSession* session);
	virtual void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message);
	
	void configure();
	static void run_client_thread(void* handle);
	
	gloox::Client* client;                                             // 客户端实例对象指针
	boost::thread* client_thread;                                      // 客户端线程
	boost::interprocess::interprocess_semaphore reconnect_semaphore;   // 立即重连信号量
	bool is_open;                                                      // 开启标识
	
	std::string server;                        // 服务器域名
	int port;                                  // 服务器端口号
	std::string account;                       // 账号名
	std::string password;                      // 密码
	std::string resource;                      // 资源名
	unsigned int reconnect_timeout;            // 重连等待时间
};

} // namespace psychokinesis
#endif
