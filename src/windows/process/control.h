#ifndef _CONTROL_H_
#define _CONTROL_H_

#include <boost/ptr_container/ptr_container.hpp>
#include "../api/api.h"

namespace psychokinesis {

class api_listener_with_control : public api_listener {
public: 
	api_listener_with_control(api* m_adapter) {
		this->m_adapter = m_adapter;
	}
	
	virtual void debug(const boost::property_tree::ptree& content) {
		// 定义专门的监听者处理调试信息
	}
	
	virtual void communicate(boost::property_tree::ptree& content) {
		content = *m_adapter->execute(content);
	}
	
private:
	api* m_adapter;
};

class control {
public:
	control();
	~control();
	
	bool open();
	void close();
	
private:
	void bind_listener(api* bind_api, api* listen_api);
	
	boost::ptr_list<api_listener> api_listener_list;
	boost::ptr_list<api> adapter_list;
};

} // namespace psychokinesis

#endif