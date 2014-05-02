#include <boost/assert.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "api_communication.h"
#include <gloox/message.h>
#include <gloox/rostermanager.h>

using std::vector;
using std::string;
using boost::thread;
using boost::shared_ptr;
using boost::property_tree::ptree;
using namespace psychokinesis;

bool api_communication::open() {
	if (is_open)
		return true;
		
	client = new gloox::Client(server);
	
	// 注册连接状态监听器，实现onConnect，onDisconnect和onTLSConnect接口
	client->registerConnectionListener(this);
	// 注册会话session监听器，当有新会话建立时会调用handleMessageSession接口
	client->registerMessageSessionHandler(this, 0);
	// 注册log监听器，实现handleLog接口
	client->logInstance().registerLogHandler(gloox::LogLevelDebug, gloox::LogAreaAll, this);
	
	configure();
	
	is_open = true;
	
	client_thread = new thread(run_client_thread, this);
	
	debug_print("api_communication open");
	
	return true;
}


shared_ptr<ptree> api_communication::execute(const ptree& args, const api* /*caller*/) {
	shared_ptr<ptree> resp(new ptree());
	
	try {
		string opr = args.get<string>("opr");
		
		if (opr == "communicate") {
			string account = args.get<string>("account");
			std::string pa(account);
			
			if (args.count("resource_name")) {
				string resource_name = args.get<string>("resource_name");
				pa = pa + "@" + server + "/" + resource_name;
			} else {
				pa = pa + "@" + server;
			}
			
			gloox::JID jid;
			if (jid.setJID(pa) == false) {
				resp->put("ret_code", 2);
				return resp;
			}

			gloox::MessageSession* session = new gloox::MessageSession(client, jid);
			
			string message = args.get<string>("message");
			session->send(message);
			
			client->disposeMessageSession(session);
			
			resp->put("ret_code", 0);
			return resp;
		} else if (opr == "configure") {
			ptree parameters = args.get_child("parameters");
			
			if (parameters.count("server"))
				server = parameters.get<string>("server");
			if (parameters.count("port"))
				port = parameters.get<int>("port");
			if (parameters.count("account"))
				account = parameters.get<string>("account");
			if (parameters.count("password"))
				password = parameters.get<string>("password");
			if (parameters.count("resource"))
				resource = parameters.get<string>("resource");
			if (parameters.count("reconnect_timeout"))
				reconnect_timeout = parameters.get<unsigned int>("reconnect_timeout");
			
			if (is_open)
				configure();
			
			resp->put("ret_code", 0);
			return resp;
		} else {
			debug_print("bad json!");
		}
		
	} catch (boost::property_tree::ptree_bad_path) {
		debug_print("bad json!");
	} catch (...) {
		debug_print("Unexpected error!");
	}
	
	resp->put("ret_code", 1);
	return resp;
}


void api_communication::close() {
	if (!is_open)
		return;
	
	is_open = false;
	if (!reconnect_semaphore.try_wait())
		reconnect_semaphore.post();
	
	client->presence().setPresence(gloox::Presence::Unavailable);
	client->setPresence();                                  // 广播离线消息
	
	client->disconnect();
	
	client_thread->join();
	delete client_thread;
	client_thread = NULL;
	
	delete client;
	client = NULL;
	
	debug_print("api_communication close");
}


shared_ptr< vector<string> > api_communication::available_resources_get(const string& account) {
	shared_ptr< vector<string> > resources = shared_ptr< vector<string> >(new vector<string>());
		
	gloox::JID jid;
	if (jid.setJID(account + "@" + server) == false) {
		return resources;
	}
	
	gloox::RosterItem* roster = client->rosterManager()->getRosterItem(jid);           // 前提为jid在roster列表中
	if (roster == NULL) {
		return resources;
	}
	
	const gloox::RosterItem::ResourceMap& resource_map = roster->resources();
	for (gloox::RosterItem::ResourceMap::const_iterator it = resource_map.begin();
		 it != resource_map.end();
		 ++it) {
		 if (it->second->presence() == gloox::Presence::Available) {
			resources->push_back(it->first);
		 }
	}
	return resources;
}


void api_communication::handleMessage(const gloox::Message& msg, gloox::MessageSession* session) {
	ptree content;
	
	content.put("info", "message");
	content.put("account", msg.from().username());
	content.put("resource_name", msg.from().resource());
	content.put("message", msg.body());
	
	shared_ptr<string> resp = communicate(*this, content);
	
	if (resp->length() > 0)
		session->send(*resp);
	
	client->disposeMessageSession(session);
}


void api_communication::onConnect() {
	ptree content;
	
	content.put("info", "connected");
	
	communicate(*this, content);

	debug_print("api_communication connect");
}


void api_communication::onDisconnect(gloox::ConnectionError e) {
	ptree content;
	
	content.put("info", "disconnect");
	content.put("error_code", e);
	
	communicate(*this, content);
	
	debug_print("api_communication disconnect");
}


bool api_communication::onTLSConnect(const gloox::CertInfo& info) {
	return true;                 // 返回true表示接受此TLS连接，如返回false则关闭此连接
}


void api_communication::handleMessageSession(gloox::MessageSession* session) {
	// 注册此会话的消息监听器，当有新消息到来时会调用handleMessage接口
	session->registerMessageHandler(this);
}


void api_communication::handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message) {
	debug_print(message);
}


void api_communication::configure() {
	BOOST_ASSERT(client && "gloox haven't open!");
	
	client->disconnect();
	
	client->setServer(server);
	const_cast<gloox::JID&>(client->jid()).setServer(server);            // 需同时修改JID里的server
	if (port > -1)
		client->setPort(port);
	client->setUsername(account);
	client->setPassword(password);
	client->setResource(resource);
	
	if (!reconnect_semaphore.try_wait() && account.length() > 0)         // 账号长度大于0时才尝试连接，防止空账号登录
		reconnect_semaphore.post();
}


void api_communication::run_client_thread(void* handle) {
	api_communication* h = (api_communication*)handle;
	
	h->reconnect_semaphore.wait();
	
	while (h->is_open) {
		// 调用 j->connect(true)时，即实现与服务器的连接，连接成功会返回真。
		// connect函数参数为false表示不阻塞方式连接，经测试也会造成短暂阻塞
		// 而如果为true ，则为阻塞方式连接，即会等待接收数据
		h->client->connect(true);
		
		if (h->is_open) {
			if (h->reconnect_timeout == 0)
				h->reconnect_semaphore.wait();
			else
				h->reconnect_semaphore.timed_wait(boost::get_system_time() + boost::posix_time::seconds(h->reconnect_timeout));
		}
	}
}



// #define TEST                                        // 编译指令： i686-w64-mingw32-g++ api_communication.cpp -o communication.exe -g -static -DPSYCHOKINESIS_DEBUG -I$HOME/mingw32/include -L$HOME/mingw32/lib -lgloox -lwsock32 -lgdi32 -ldnsapi -lcrypt32 -lsecur32 -lboost_system -lboost_thread_win32
#ifdef TEST

#include <iostream>
#include <boost/property_tree/json_parser.hpp>

using namespace std;

class api_communication_listener : public api_listener {
public:
	virtual void debug(const ptree& content) {
		try {
			cout << "debug: " << content.get<string>("debug") << endl;
		} catch (boost::property_tree::ptree_bad_path) {
			cout << "bad json!" << endl;
			BOOST_ASSERT(0 && "");
		}
	}
	
	virtual void communicate(const api& caller, ptree& content) {
		stringstream content_str;
		write_json(content_str, content);
		
		cout << "communicate: " << content_str.str() << endl;
		
		content.clear();
	}
};

int main() {
	api_communication api;
	api_communication_listener listener;
	char json_path[260];
	ptree json;
	shared_ptr<ptree> result;
	
	cout << "config json file's path: ";
	cin.getline(json_path, 260);
	cout << json_path << endl;
	if (cin.eof())
		return 0;
		
	try {
		boost::property_tree::read_json(json_path, json);
	} catch (...) {
		cout << "parse config json file failed!" << endl;
		return 0;
	}
		
	result = api.execute(json);
	
	if (result->get<int>("ret_code")) {
		cout << "config failed! ret_code = " << result->get<int>("ret_code") << endl;
		return 0;
	}
	
	if (api.open() == false) {
		cout << "open gloox failed!" << endl;
		return 0;
	}
	api.attach_listener(&listener);
	
	while (true) {
		cout << "json file's path: ";
		cin.getline(json_path, 260);
		cout << json_path << endl;
		if (cin.eof())
			break;
		
		try {
			boost::property_tree::read_json(json_path, json);
		} catch (...) {
			cout << "parse json file failed!" << endl;
			continue;
		}
		
		result = api.execute(json);
		
		std::stringstream result_str;
		write_json(result_str, *result);
		cout << "the result: " << endl
			 << result_str.str() << endl;
	}
	
	api.close();
	api.detach_listener(&listener);
	
	return 0;
}

#endif
