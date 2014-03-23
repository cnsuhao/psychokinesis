#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "adapter_communication.h"

#define LOGIN_SERVER_INFO_FILE "server.json"

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


shared_ptr<ptree> adapter_communication::execute(const ptree& args) {
	ptree req;
	
	// TODO 需要填写接收账号
	req.put("opr", "communicate");
	req.add_child("content", args);
	
	return m_api->execute(req);
}
