#include <DuiLib/StdAfx.h>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <time.h>
#include "control.h"
#include "ui_control.h"
#include "../api/api_download.h"
#include "../api/api_communication.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"
#include "../ui/frame_window.h"
#include "../ui/api_message.h"
#include "../ui/encoding_changer.h"

#define CONFIG_FILE "config.xml"

using std::string;
using boost::property_tree::ptree;
using DuiLib::CPaintManagerUI;

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
		char strdate[32];
		time_t timep;
		time(&timep);
		strftime(strdate, sizeof(strdate), "%X", localtime(&timep));
		
		try {
			debug_file << "debug(" << strdate << "): " << content.get<string>("debug") << std::endl;
		} catch (boost::property_tree::ptree_bad_path) {
			BOOST_ASSERT(0 && "bad debug json!");
		}
		
		debug_file.flush();
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
	ptree config;
	
	try {
		ptree root;
		boost::property_tree::read_xml(CONFIG_FILE, root);
		config = root.get_child("config");
	} catch (...) {
		MessageBox(NULL, _T("找不到所需的配置文件！请尝试重新下载安装软件。"), _T("错误"), MB_ICONERROR | MB_OK);
		::exit(0);
		return;
	}
	
	// 设置api_communication
	ptree communication_config;
	bool immediate_connect = true;
	string account, password;
	try {
		communication_config = config.get_child("communication");
		account = communication_config.get<string>("account");
		password = communication_config.get<string>("password");
	} catch (...) {
		immediate_connect = false;
	}
	
	if (account.length() == 0)
		immediate_connect = false;
	
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	if (immediate_connect) {
		dynamic_cast<adapter_communication*>(&(*communication_adapter))->immediate_connect_set(true);
		
		frame_window& m_window = frame_window::get_mutable_instance();
		m_window.post_message(new api_communication_logging(account, password));
	}
	
	ptree json;
	json.put_child("parameters", communication_config);
	json.put("opr", "configure");
	communication_adapter->execute(json);
	
	// 设置api_download（utf8编码->ascii编码）
	ptree download_config;
	aria2::KeyVals options;
	bool has_store_path = false;
	try {
		download_config = config.get_child("download");
	} catch (...) 
	{}
	
	BOOST_FOREACH(const ptree::value_type& option, download_config) {
		options.push_back(std::make_pair(option.second.get<string>("name"),
										 encoding_changer::utf82ascii(option.second.get<string>("value"))));
		
		if (option.second.get<string>("name") == "dir" && 
			option.second.get<string>("value").length() > 0)
			has_store_path = true;
	}
	
	if (!has_store_path) {
		string store_path = CPaintManagerUI::GetInstancePath().GetData();
		options.push_back(std::make_pair("dir", store_path.c_str()));
	}
	
	boost::ptr_list<api>::iterator download_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_download>());
	adapter_download* download = dynamic_cast<adapter_download*>(&(*download_adapter));
	
	download->change_global_option(options);
}


void control::save_config() {
	ptree config;
	
	// 保存api_communication的配置
	ptree communication_config;
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	adapter_communication* communication = dynamic_cast<adapter_communication*>(&(*communication_adapter));
	
	communication_config.put("server", communication->server_ip_get());
	communication_config.put("port", communication->server_port_get());
	communication_config.put("account", communication->account_get());
	communication_config.put("password", communication->password_get());
	communication_config.put("resource", communication->resource_get());
	
	config.put_child("communication", communication_config);
	
	// 保存api_download的配置（ascii编码->utf8编码）
	ptree download_config;
	boost::ptr_list<api>::iterator download_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_download>());
	adapter_download* download = dynamic_cast<adapter_download*>(&(*download_adapter));
	
	aria2::KeyVals options = download->get_global_options();
	BOOST_FOREACH(const aria2::KeyVals::value_type& option, options) {
		ptree item;
		item.put("name", option.first);
		item.put("value", encoding_changer::ascii2utf8(option.second));
		download_config.push_back(std::make_pair("KeyVals", item));
	}
	
	config.put_child("download", download_config);
	
	
	boost::filesystem::path file(CONFIG_FILE);
    if(boost::filesystem::exists(file))
            boost::filesystem::remove(file);
	
	ptree root;
	root.put_child("config", config);
	boost::property_tree::write_xml(CONFIG_FILE, root);
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