#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/ptr_container/ptr_unordered_map.hpp>
#include "../api/api.h"

namespace psychokinesis {

class control {
public:
	control(boost::shared_ptr<boost::ptr_unordered_map<std::string, api>>& api_list)
	 : api_list(api_list)
	{}
	
	int handle_message(const std::string& req, std::string& resp);
	
private:
	boost::weak_ptr<boost::ptr_unordered_map<std::string, api>> api_list;
};

} // namespace psychokinesis

#endif