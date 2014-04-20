#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <boost/ptr_container/ptr_container.hpp>
#include <boost/serialization/singleton.hpp>
#include "../api/api.h"
#include "config_control.h"


namespace psychokinesis {

template <typename T>
class find_api_func {
public:
	bool operator()(const api& m_api) {
		if (typeid(m_api) == typeid(T))
			return true;
		else
			return false;
	}
};

class api_listener_with_control : public api_listener {
public: 
	api_listener_with_control(api* m_adapter) {
		this->m_adapter = m_adapter;
	}
	
	virtual void debug(const boost::property_tree::ptree& content) {
		// 定义专门的监听者处理调试信息
	}
	
	virtual void communicate(const api& caller, boost::property_tree::ptree& content) {
		content = *m_adapter->execute(content, &caller);
	}
	
private:
	api* m_adapter;
};

class ui_control;

class control : public boost::serialization::singleton<control> {
public:
	control();
	~control();
	
	friend class ui_control;
	friend class config_control;
	
	bool open();
	void close();
	
private:
	void bind_listener(api* bind_api, api* listen_api);
	bool load_config();
	void save_config();
	
	boost::ptr_list<api_listener> api_listener_list;
	boost::ptr_list<api> adapter_list;
	
	config_control config;
};

} // namespace psychokinesis

#endif