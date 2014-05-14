#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <time.h>
#include "control.h"
#include "ui_control.h"
#include "config_control.h"
#include "../api/api_download.h"
#include "../api/api_communication.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"

using std::string;
using boost::property_tree::ptree;

using namespace psychokinesis;

#ifdef PSYCHOKINESIS_DEBUG
#include <stdio.h>

class api_listener_with_debug : public api_listener {
public:
	api_listener_with_debug() {
		debug_file = fopen("log.txt", "a+");
	}
	
	~api_listener_with_debug() {
		fclose(debug_file);
	}
	
	virtual void debug(const ptree& content) {
		char strdate[32];
		time_t timep;
		time(&timep);
		strftime(strdate, sizeof(strdate), "%X", localtime(&timep));
		
		try {
			fprintf(debug_file, "%-10s%s\n", strdate, content.get<string>("debug").c_str());
		} catch (boost::property_tree::ptree_bad_path) {
			BOOST_ASSERT(0 && "bad debug json!");
		}
		
		fflush(debug_file);
	}
	
	virtual boost::shared_ptr<ptree> communicate(const api& caller, const ptree& content) {
		return boost::shared_ptr<ptree>(new ptree());
	}
	
private:
	FILE* debug_file;
};
#endif

control::control() {
	ui_control& m_ui_control = ui_control::get_mutable_instance();
	
	api_download* download = new api_download();
	adapter_download* download_adapter = new adapter_download(download);
	api_communication* communication = new api_communication();
	adapter_communication* communication_adapter = new adapter_communication(communication);
	
	// api_download的事件监听者
	bind_listener(download, communication_adapter);
	
	// api_communication的事件监听者
	bind_listener(communication, download_adapter);
	
	download->attach_listener(&m_ui_control);
	communication->attach_listener(&m_ui_control);
	
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
	// 获取配置设置API
	if (!load_config())
		return false;
	
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


bool control::load_config() {
	return config_control::get_mutable_instance().load_config();
}


void control::save_config() {
	config_control::get_mutable_instance().save_config();
}


// #define TEST
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