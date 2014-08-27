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
	
	void logout() {
		m_api->logout();
	}
	
	const std::string& server_ip_get() const {
		return m_api->server_ip_get();
	}
	
	void server_ip_set(const std::string& server_ip) {
		m_api->server = server_ip;
	}
	
	int server_port_get() const {
		return m_api->server_port_get();
	}
	
	void server_port_set(int server_port) {
		m_api->port = server_port;
	}
	
	const std::string& account_get() const {
		return m_api->account_get();
	}
	
	void account_set(const std::string& account) {
		m_api->account = account;
	}
	
	const std::string& password_get() const {
		return m_api->password_get();
	}
	
	void password_set(const std::string& password) {
		m_api->password = password;
	}
	
	const std::string& resource_get() const {
		return m_api->resource_get();
	}
	
	void resource_set(const std::string& resource) {
		m_api->resource = resource;
	}
	
	void reconnect_timeout_set(unsigned int reconnect_timeout) {
		m_api->reconnect_timeout_set(reconnect_timeout);
	}
	
private:
	boost::shared_ptr<api_communication> m_api;
};

} // namespace psychokinesis
#endif
