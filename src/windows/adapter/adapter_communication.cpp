#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "adapter_communication.h"
#include "../api/api_download.h"

using std::vector;
using std::string;
using boost::shared_ptr;
using boost::property_tree::ptree;
using namespace psychokinesis;


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
