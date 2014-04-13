#ifndef _UI_CONTROL_H_
#define _UI_CONTROL_H_

#include <string>
#include <boost/serialization/singleton.hpp>
#include <boost/ptr_container/ptr_container.hpp>
#include "../api/api.h"

namespace psychokinesis {

class ui_control : public api_listener, public boost::serialization::singleton<ui_control> {
public:
	void login(const std::string& account, const std::string& password);
	
	virtual void debug(const boost::property_tree::ptree& content)
	{}
	
	virtual void communicate(const api& caller, boost::property_tree::ptree& content);
private:
	void on_logged();
	void on_login_failed(int error_code);
};

} // namespace psychokinesis

#endif