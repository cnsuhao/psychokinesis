#include <sstream>
#include <boost/assert.hpp>
#include "control.h"
#include "ui_control.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"
#include "../ui/frame_window.h"
#include "../ui/api_message.h"

using std::string;
using std::stringstream;
using boost::property_tree::ptree;

using namespace psychokinesis;

void ui_control::login(const string& account, const string& password) {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	
	ptree args, parameters;
	parameters.put("account", account);
	parameters.put("password", password);
	parameters.put("reconnect_timeout", 0);             // 不进行自动重连
	args.put("opr", "configure");
	args.add_child("parameters", parameters);
	
	communication_adapter->execute(args, NULL);
	
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_communication_logging(account, password));
}


void ui_control::change_store_path(const std::string& path) {
	change_download_global_option("dir", path);
}


void ui_control::change_max_download_limit(unsigned int limit) {
	stringstream str_limit;
	str_limit << limit;
	change_download_global_option("max-overall-download-limit", str_limit.str());
}


void ui_control::change_max_upload_limit(unsigned int limit) {
	stringstream str_limit;
	str_limit << limit;
	change_download_global_option("max-overall-upload-limit", str_limit.str());
}


boost::shared_ptr<ptree> ui_control::communicate(const api& caller, const ptree& content) {
	string info;
		
	try {
		info = content.get<string>("info");
	} catch(...) {
		return boost::shared_ptr<ptree>(new ptree());
	}
	
	if (typeid(caller) == typeid(api_communication)) {
		if (info == "connected") {
			on_logged();
		}
		else if (info == "disconnect") {
			int error_code = content.get<int>("error_code");
			on_login_failed(error_code);
		}
	} else if (typeid(caller) == typeid(api_download)) {
		if (info == "global_options_changed") {
			ptree option_list = content.get_child("options");
			BOOST_FOREACH(const ptree::value_type& option, option_list) {
				string option_name = option.second.get<string>("name");
				
				if (option_name == "dir") {
					on_download_store_path_changed(option.second.get<string>("value"));
				} else if (option_name == "max-overall-download-limit") {
					on_download_max_download_limit_changed(option.second.get<unsigned int>("value"));
				} else if (option_name == "max-overall-upload-limit") {
					on_download_max_upload_limit_changed(option.second.get<unsigned int>("value"));
				}
			}
		}
	}
	
	return boost::shared_ptr<ptree>(new ptree());
}


inline void ui_control::change_download_global_option(const string& name, const string& value) {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	boost::ptr_list<api>::iterator download_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_download>());
	adapter_download* download = dynamic_cast<adapter_download*>(&(*download_adapter));																	
	
	aria2::KeyVals option;
	option.push_back(std::make_pair(name.c_str(), value.c_str()));
	download->change_global_option(option);
}


void ui_control::on_logged() {
	control::get_mutable_instance().save_config();
	
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		find_api_func<adapter_communication>());
	adapter_communication* communication = dynamic_cast<adapter_communication*>(&(*communication_adapter));
	
	communication->reconnect_timeout_set(10);                      // 登录成功后设置中断后重连时间为10s
	
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_communication_logged(communication->account_get(), communication->password_get()));
}


void ui_control::on_login_failed(int error_code) {
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_communication_login_failed(error_code));
}


void ui_control::on_download_store_path_changed(const std::string& new_path) {
	control::get_mutable_instance().save_config();
	
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_download_store_path_changed(new_path));
}


void ui_control::on_download_max_download_limit_changed(unsigned int limit) {
	control::get_mutable_instance().save_config();
	
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_download_max_download_limit_changed(limit));
}


void ui_control::on_download_max_upload_limit_changed(unsigned int limit) {
	control::get_mutable_instance().save_config();
	
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_download_max_upload_limit_changed(limit));
}
