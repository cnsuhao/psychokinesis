#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include "control.h"
#include "../api/api_download.h"
#include "../api/api_communication.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"

using namespace psychokinesis;

#ifdef PSYCHOKINESIS_DEBUG
#include <iostream>

using namespace std;

class api_listener_with_debug : public api_listener {
public:
	virtual void debug(const boost::property_tree::ptree& content) {
		try {
			cout << "debug: " << content.get<string>("debug") << endl;
		} catch (boost::property_tree::ptree_bad_path) {
			cout << "bad json!" << endl;
			BOOST_ASSERT(0 && "");
		}
	}
	
	virtual void communicate(boost::property_tree::ptree& content) {
	}
};
#endif

control::control() {
	api_download* download = new api_download();
	adapter_download* download_adapter = new adapter_download(download);
	api_communication* communication = new api_communication();
	adapter_communication* communication_adapter = new adapter_communication(communication);
	
	// api_download的事件监听者
	bind_listener(download, communication_adapter);
	
	// api_communication的事件监听者
	bind_listener(communication, download_adapter);
	
#ifdef PSYCHOKINESIS_DEBUG
	api_listener* debug_listener = new api_listener_with_debug();
	
	download->attach_listener(debug_listener);
	communication->attach_listener(debug_listener);
	api_listener_list.push_back(debug_listener);
#endif

	adapter_list.push_back(download_adapter);
	adapter_list.push_back(communication_adapter);
}


control::~control() {
	close();
}


bool control::open() {
	BOOST_FOREACH(api& i, adapter_list) {
		if (!i.open())
		{
			close();
			return false;
		}
	}
	
	return true;
}


void control::close() {
	BOOST_FOREACH(api& i, adapter_list) {
		i.close();
	}
}


void control::bind_listener(api* bind_api, api* listen_api) {
	api_listener* listener = new api_listener_with_control(listen_api);
	bind_api->attach_listener(listener);
	api_listener_list.push_back(listener);
}

#define TEST
#ifdef TEST

#include <iostream>

using namespace std;

int main() {
	control m_control;
	
	if (!m_control.open()) {
		cout << "control open failed" << endl;
		return 0;
	}
	
	cout << "control working..." << endl;
	cout << "press any key to close" << endl;
	
	getchar();
	
	m_control.close();
	
	return 0;
}

#endif