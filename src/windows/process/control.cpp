#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include "control.h"
#include "ui_control.h"
#include "../api/api_download.h"
#include "../api/api_communication.h"
#include "../adapter/adapter_download.h"
#include "../adapter/adapter_communication.h"

using namespace psychokinesis;

#ifdef PSYCHOKINESIS_DEBUG
#include <fstream>

class api_listener_with_debug : public api_listener {
public:
	api_listener_with_debug() {
		debug_file.open("log.txt", std::ofstream::out | std::ofstream::app);
	}
	
	~api_listener_with_debug() {
		debug_file.close();
	}
	
	virtual void debug(const boost::property_tree::ptree& content) {
		try {
			debug_file << "debug: " << content.get<std::string>("debug") << std::endl;
		} catch (boost::property_tree::ptree_bad_path) {
			BOOST_ASSERT(0 && "bad debug json!");
		}
	}
	
	virtual void communicate(const api& caller, boost::property_tree::ptree& content) {
	}
	
private:
	std::ofstream debug_file;
};
#endif

control::control() {
	ui_control& m_ui_control = ui_control::get_mutable_instance();
	
	api_download* download = new api_download();
	adapter_download* download_adapter = new adapter_download(download);
	api_communication* communication = new api_communication();
	adapter_communication* communication_adapter = new adapter_communication(communication);
	
	// UI������Ҫ������ȼ���������֤��Ϣû�ı�
	download->attach_listener(&m_ui_control);
	communication->attach_listener(&m_ui_control);
	
	// ��ı���Ϣ�ļ��������ȼ����
	// api_download���¼�������
	bind_listener(download, communication_adapter);
	
	// api_communication���¼�������
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