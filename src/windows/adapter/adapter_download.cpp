#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include "adapter_download.h"

using std::string;
using std::stringstream;
using boost::shared_ptr;
using boost::property_tree::ptree;
using namespace psychokinesis;

shared_ptr<ptree> adapter_download::execute(const ptree& args, const api* caller) {
	shared_ptr<ptree> resp(new ptree());
	string method, session_id;
	ptree message;
	
	try {
		std::stringstream message_str(args.get<string>("message"));
		
		m_api->debug_print(message_str.str());
		
		boost::property_tree::read_json(message_str, message);
		method = message.get<string>("method");
		session_id = message.get<string>("session_id");
	} catch (boost::property_tree::json_parser_error) {
		return resp;
	} catch (boost::property_tree::ptree_bad_path) {
		return resp;
	}catch (...) {
		m_api->debug_print("Unexpected error!");
	}
	
	if (method != "download")
		return resp;
		
	resp = m_api->execute(message);
	
	resp->put("session_id", session_id);
	return resp;
}
