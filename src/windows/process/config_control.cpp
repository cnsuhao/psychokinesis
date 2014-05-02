#include <DuiLib/StdAfx.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "config_control.h"
#include "control.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"
#include "../ui/frame_window.h"
#include "../ui/api_message.h"
#include "../ui/encoding_changer.h"

#define CONFIG_FILE "config.xml"

using std::string;
using boost::shared_ptr;
using boost::property_tree::ptree;
using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

bool config_control::load_config() {
	ptree config;
	
	try {
		ptree root;
		boost::property_tree::read_xml(CONFIG_FILE, root);
		config = root.get_child("config");
	} catch (...) {
		return false;
	}
	
	communication_load(config);
	download_load(config);
	
	return true;
}


void config_control::save_config() {
	ptree config;
	
	shared_ptr<ptree> communication_config = communication_save();
	config.put_child("communication", *communication_config);
	
	shared_ptr<ptree> download_config = download_save();
	config.put_child("download", *download_config);
	
	
	boost::filesystem::path file(CONFIG_FILE);
    if(boost::filesystem::exists(file))
            boost::filesystem::remove(file);
	
	ptree root;
	root.put_child("config", config);
	boost::property_tree::write_xml(CONFIG_FILE, root);
}



void config_control::communication_load(const ptree& config) {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
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
}


shared_ptr<ptree> config_control::communication_save() {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	shared_ptr<ptree> communication_config = shared_ptr<ptree>(new ptree());
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	adapter_communication* communication = dynamic_cast<adapter_communication*>(&(*communication_adapter));
	
	communication_config->put("server", communication->server_ip_get());
	communication_config->put("port", communication->server_port_get());
	communication_config->put("account", communication->account_get());
	communication_config->put("password", communication->password_get());
	communication_config->put("resource", communication->resource_get());
	
	return communication_config;
}


void config_control::download_load(const ptree& config) {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	ptree download_config;
	aria2::KeyVals options;
	bool has_store_path = false, has_save_session = false, has_continue = false, has_max_connection_per_server = false,
		 has_dht_dat = false;
	try {
		download_config = config.get_child("download");
	} catch (...) 
	{}
	
	BOOST_FOREACH(const ptree::value_type& option, download_config) {
		string value = option.second.get<string>("value");
		options.push_back(std::make_pair(option.second.get<string>("name"), value));
		
		if (option.second.get<string>("name") == "dir" && 
			value.length() > 0) {
			// 确保store_path存在
			boost::filesystem::path dir(encoding_changer::utf82ascii(value));
			boost::filesystem::create_directories(dir);
			
			has_store_path = true;
		}
		
		if (option.second.get<string>("name") == "save-session" && 
			value.length() > 0) {
			// 从save-session文件读取上次运行的任务
			options.push_back(std::make_pair("input-file", value));
			has_save_session = true;
		}
		
		if (option.second.get<string>("name") == "continue" && 
			value.length() > 0)
			has_continue = true;
		
		if (option.second.get<string>("name") == "max-connection-per-server" && 
			value.length() > 0)
			has_max_connection_per_server = true;
			
		if (option.second.get<string>("name") == "dht-file-path" && 
			value.length() > 0)
			has_dht_dat = true;
	}
	
	// 设置api_download的默认值
	string instance_path = encoding_changer::ascii2utf8(CPaintManagerUI::GetInstancePath().GetData());
	if (!has_store_path) {
		options.push_back(std::make_pair("dir", instance_path));
	}
	if (!has_save_session) {
		string session_file = instance_path + "session.dat";
		std::ofstream(session_file.c_str(), std::ios::out);
		
		options.push_back(std::make_pair("save-session", session_file));
		options.push_back(std::make_pair("save-session-interval", "60"));
	}
	if (!has_continue) {
		options.push_back(std::make_pair("continue", "true"));               // 断点续传
	}
	if (!has_max_connection_per_server) {
		options.push_back(std::make_pair("max-connection-per-server", "5"));
	}
	if (!has_dht_dat) {
		options.push_back(std::make_pair("dht-file-path", instance_path + "dht.dat"));
		options.push_back(std::make_pair("enable-dht", "true"));
		options.push_back(std::make_pair("dht-entry-point", "dht.transmissionbt.com:6881"));         // 指定dht网络入口点，解决bt下载无速度的问题
	}
#ifdef PSYCHOKINESIS_DEBUG
	options.push_back(std::make_pair("log-level", "debug"));
	options.push_back(std::make_pair("log", instance_path + "aria2.log"));
#endif
	
	boost::ptr_list<api>::iterator download_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_download>());
	adapter_download* download = dynamic_cast<adapter_download*>(&(*download_adapter));
	
	download->change_global_option(options);
}


shared_ptr<ptree> config_control::download_save() {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	shared_ptr<ptree> download_config = shared_ptr<ptree>(new ptree());
	boost::ptr_list<api>::iterator download_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_download>());
	adapter_download* download = dynamic_cast<adapter_download*>(&(*download_adapter));
	
	aria2::KeyVals options = download->get_global_options();
	BOOST_FOREACH(const aria2::KeyVals::value_type& option, options) {
		ptree item;
		item.put("name", option.first);
		item.put("value", option.second);
		download_config->push_back(std::make_pair("KeyVals", item));
	}
	
	return download_config;
}
