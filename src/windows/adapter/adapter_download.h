#ifndef _ADAPTER_DOWNLOAD_H_
#define _ADAPTER_DOWNLOAD_H_

#include "../api/api_download.h"

namespace psychokinesis {

class adapter_download : public api {
public:
	adapter_download(api_download* m_api)
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
	
	void change_global_option(const aria2::KeyVals& options) {
		m_api->change_global_option(options);
	}
	
	aria2::KeyVals get_global_options() {
		return m_api->get_global_options();
	}

private:
	boost::shared_ptr<api_download> m_api;
};

} // namespace psychokinesis
#endif