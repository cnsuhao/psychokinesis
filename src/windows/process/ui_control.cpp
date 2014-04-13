#include <boost/assert.hpp>
#include "control.h"
#include "ui_control.h"
#include "../adapter/adapter_communication.h"
#include "../ui/frame_window.h"
#include "../ui/api_message.h"

using std::string;
using boost::property_tree::ptree;

using namespace psychokinesis;

template <typename T>
class list_find_func {
public:
	bool operator()(const api& m_api) {
		if (typeid(m_api) == typeid(T))
			return true;
		else
			return false;
	}
};

void ui_control::login(const string& account, const string& password) {
	boost::ptr_list<api>& adapter_list = control::get_mutable_instance().adapter_list;
	boost::ptr_list<api>::iterator communication_adapter = std::find_if(adapter_list.begin(),
																		adapter_list.end(), 
																		list_find_func<adapter_communication>());
	if (communication_adapter == adapter_list.end())
		BOOST_ASSERT(0 && "communication_adapter can't be found!");
		
	ptree args, parameters;
	parameters.put("server", "127.0.0.1");
	parameters.put("port", "5223");
	parameters.put("account", account);
	parameters.put("password", password);
	parameters.put("resource", "psychokinesis-pc");
	parameters.put("reconnect_timeout", 10);
	args.put("opr", "configure");
	args.add_child("parameters", parameters);
	
	communication_adapter->execute(args, NULL);
}


void ui_control::communicate(const api& caller, boost::property_tree::ptree& content) {
	if (typeid(caller) == typeid(api_communication)) {
		string info;
		
		try {
			info = content.get<string>("info");
		} catch(...) {
			BOOST_ASSERT(0 && "communication_api communicate failed!");
		}
		
		if (info == "connected") {
			on_logged();
		}
		else if (info == "disconnect") {
			int error_code = content.get<int>("error_code");
			on_login_failed(error_code);
		}
	}
}


void ui_control::on_logged() {
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_communication_logged());
}


void ui_control::on_login_failed(int error_code) {
	frame_window& m_window = frame_window::get_mutable_instance();
	m_window.post_message(new api_communication_login_failed(error_code));
}
