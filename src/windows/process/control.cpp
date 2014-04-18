#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include "control.h"
#include "ui_control.h"
#include "../api/api_download.h"
#include "../api/api_communication.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"

#define LOGIN_SERVER_INFO_FILE "server.json"

using boost::property_tree::ptree;

using namespace psychokinesis;

#ifdef PSYCHOKINESIS_DEBUG
#include <fstream>

class api_listener_with_debug : public api_listener {
public:
	api_listener_with_debug() {
		debug_file.open("log.txt", std::ofstream::out | std::ofstream::app);
	}
	
	~api_listener_with_debug() {
		debug_file.close();
	}
	
	virtual void debug(const boost::property_tree::ptree& content) {
		try {
			debug_file << "debug: " << content.get<std::string>("debug") << std::endl;
		} catch (boost::property_tree::ptree_bad_path) {
			BOOST_ASSERT(0 && "bad debug json!");
		}
	}
	
	virtual void communicate(const api& caller, boost::property_tree::ptree& content) {
	}
	
private:
	std::ofstream debug_file;
};
#endif

control::control() {
	ui_control& m_ui_control = ui_control::get_mutable_instance();
	
	api_download* download = new api_download();
	adapter_download* download_adapter = new adapter_download(download);
	api_communication* communication = new api_communication();
	adapter_communication* communication_adapter = new adapter_communication(communication);
	
	// UI控制需要最高优先级监听，保证消息没改变
	download->attach_listener(&m_ui_control);
	communication->attach_listener(&m_ui_control);
	
	// 会改变消息的监听者优先级最低
	// api_download的事件监听者
	bind_listener(download, communication_adapter);
	
	// api_communication的事件监听者
	bind_listener(communication, download_adapter);
	
#ifdef PSYCHOKINESIS_DEBUG
	api_listener* debug_listener = new api_listener_with_debug();
	
	download->attach_listener(debug_listener);
	communication->attach_listener(debug_listener);
	api_listener_list.push_back(debug_listener);
#endif

	adapter_list.push_back(download_adapter);
	adapter_list.push_back(communication_adapter);
}


control::~control() {
	close();
}


bool control::open() {
	// 获取配置设置API
	load_config();
	
	BOOST_FOREACH(api& i, adapter_list) {
		if (!i.open())
		{
			close();
			return false;
		}
	}
	
	return true;
}


void control::close() {
	BOOST_FOREACH(api& i, adapter_list) {
		i.close();
	}
}


void control::bind_listener(api* bind_api, api* listen_api) {
	api_listener* listener = new api_listener_with_control(listen_api);
	bind_api->attach_listener(listener);
	api_listener_list.push_back(listener);
}


void control::load_config() {
	ptree json;
	bool immediate_connect = true;
	
	try {
		boost::property_tree::read_json(LOGIN_SERVER_INFO_FILE, json);
	} catch (...) {
		immediate_connect = false;
	}
	
	// 设置api_communication
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	if (immediate_connect) {
		dynamic_cast<adapter_communication*>(&(*communication_adapter))->immediate_connect_set(true);
		communication_adapter->execute(json);
	}
}


void control::save_config() {
	ptree config, communication_config;
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	adapter_communication* communication = dynamic_cast<adapter_communication*>(&(*communication_adapter));
	
	communication_config.put("server", communication->server_ip_get());
	communication_config.put("port", communication->server_port_get());
	communication_config.put("account", communication->account_get());
	communication_config.put("password", communication->password_get());
	communication_config.put("resource", communication->resource_get());
	communication_config.put("reconnect_timeout", 0);
	
	config.put_child("parameters", communication_config);
	config.put("opr", "configure");
	
	boost::filesystem::path file(LOGIN_SERVER_INFO_FILE);
    if(boost::filesystem::exists(file))
            boost::filesystem::remove(file);
	
	boost::property_tree::write_json(LOGIN_SERVER_INFO_FILE, config);
}


// #define TEST
#ifdef TEST

#include <iostream>

using namespace std;

int main() {
	control m_control;
	
	if (!m_control.open()) {
		cout << "control open failed" << endl;
		return 0;
	}
	
	cout << "control working..." << endl;
	cout << "press any key to close" << endl;
	
	getchar();
	
	m_control.close();
	
	return 0;
}

#endif