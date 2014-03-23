#ifndef _ADAPTER_COMMUNICATION_H_
#define _ADAPTER_COMMUNICATION_H_

#include "../api/api_communication.h"

namespace psychokinesis {

class adapter_communication : public api {
public:
	adapter_communication(api_communication* m_api)
		: m_api(m_api)
	{}
	
	virtual bool open();
	
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args);
	
	virtual void close() {
		m_api->close();
	}
	
private:
	boost::shared_ptr<api_communication> m_api;
};

} // namespace psychokinesis
#endif
