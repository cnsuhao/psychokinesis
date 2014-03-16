#include <sstream>
#include <boost/assert.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "control.h"

using std::string;
using std::stringstream;
using boost::shared_ptr;
using namespace boost::property_tree;
using namespace psychokinesis;

int control::handle_message(const string& req, string& resp)
{
	stringstream req_str(req);
	ptree req_ptree, req_content;
	string req_api;
	
	try {      
		read_json(req_str, req_ptree);
		
		req_api = req_ptree.get<string>("api");
		req_content = pt.get_child("content");
	} catch (boost::property_tree::xml_parser_error) {
		// 解析错误
		return 1;
	} catch (boost::property_tree::ptree_bad_path) {
		// 参数不存在
		return 2;
	}
	
	if (api_list->count(req_api) == 0) {
		// 请求api不存在
		return 3;
	}
	
	if ((*api_list)[req_api].open()) {
		// 打开API接口失败
		return 4;
	}
	
	shared_ptr<ptree> resp_ptree = (*api_list)[req_api].execute(req_content);
													
	stringstream resp_str;  
	write_json(resp_str, *resp_ptree);
	
	return 0;
}