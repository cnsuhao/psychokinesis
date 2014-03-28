#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "adapter_communication.h"
#include "../api/api_download.h"

#define LOGIN_SERVER_INFO_FILE "server.json"

using std::vector;
using std::string;
using boost::shared_ptr;
using boost::property_tree::ptree;
using namespace psychokinesis;

bool adapter_communication::open() {
	ptree json;
	
	try {
		boost::property_tree::read_json(LOGIN_SERVER_INFO_FILE, json);
	} catch (...) {
		m_api->debug_print("parse config json file failed!");
		return false;
	}
	
	shared_ptr<ptree> result = m_api->execute(json);
	
	if (result->get<int>("ret_code")) {
		std::stringstream ret_str;
		ret_str << "config failed! ret_code = " << result->get<int>("ret_code");
		m_api->debug_print(ret_str.str());
		return false;
	}
	
	return m_api->open();
}


shared_ptr<ptree> adapter_communication::execute(const ptree& args, const api* caller) {
	if (caller == NULL)
		return m_api->execute(args);
		
	ptree req;
	shared_ptr<ptree> ret;
	
	if (typeid(*caller) == typeid(api_download)) {
		req.put("opr", "communicate");
		
		std::stringstream message_sstr;
		boost::property_tree::write_json(message_sstr, args);
		
		req.put("account", m_api->account_get());
		req.put("message", message_sstr.str());
		
		shared_ptr< vector<string> > resources = m_api->available_resources_get(m_api->account_get());
		for (vector<string>::iterator it = resources->begin();
			 it != resources->end();
			 ++it) {
			req.put("resource_name", *it);
			
			ret = m_api->execute(req);                  // 向自己账号的所有其他在线客户端发送消息
			if (ret->get<int>("ret_code") != 0) {
				break;
			}
		}
	} else {
		req = args;
	}
	
	return ret;
}
