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
	void change_store_path(const std::string& path);
	void change_max_download_speed(const std::string& speed);
	void change_max_upload_speed(const std::string& speed);
	
	virtual void debug(const boost::property_tree::ptree& content)
	{}
	
	virtual void communicate(const api& caller, boost::property_tree::ptree& content);
private:
	inline void change_download_global_option(const std::string& name, const std::string& value);
	
	void on_logged();
	void on_login_failed(int error_code);
	void on_download_store_path_changed(const std::string& new_path);
};

} // namespace psychokinesis

#endif