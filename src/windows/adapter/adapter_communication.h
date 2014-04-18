#ifndef _ADAPTER_COMMUNICATION_H_
#define _ADAPTER_COMMUNICATION_H_

#include "../api/api_communication.h"

namespace psychokinesis {

class adapter_communication : public api {
public:
	adapter_communication(api_communication* m_api)
		: m_api(m_api)
	{}
	
	virtual bool open() {
		return m_api->open();
	}
	
	virtual boost::shared_ptr<boost::property_tree::ptree> 
		execute(const boost::property_tree::ptree& args, const api* caller);
	
	virtual void close() {
		m_api->close();
	}
	
	const std::string& server_ip_get() const {
		return m_api->server_ip_get();
	}
	
	int server_port_get() const {
		return m_api->server_port_get();
	}
	
	const std::string& account_get() const {
		return m_api->account_get();
	}
	
	const std::string& password_get() const {
		return m_api->password_get();
	}
	
	const std::string& resource_get() const {
		return m_api->resource_get();
	}
	
	void immediate_connect_set(bool immediate_connect) {
		m_api->immediate_connect_set(immediate_connect);
	}
	
	void reconnect_timeout_set(unsigned int reconnect_timeout) {
		m_api->reconnect_timeout_set(reconnect_timeout);
	}
	
private:
	boost::shared_ptr<api_communication> m_api;
};

} // namespace psychokinesis
#endif
