#include <boost/foreach.hpp>
#include "api_download.h"

using std::vector;
using std::string;
using boost::thread;
using boost::shared_ptr;
using boost::property_tree::ptree;
using namespace psychokinesis;

bool api_download::open() {
	if (is_open)
		return true;
	
	aria2::libraryInit();
	
	do {
		aria2::SessionConfig config;
		
		config.keepRunning = true;
		config.downloadEventCallback = downloadEventCallback;
		config.userData = this;
		
		aria2_session = aria2::sessionNew(aria2_global_options, config);
		if (aria2_session == NULL)
			break;
			
		aria2_thread = new thread(run_aria2, this);
		
		debug_print("api_download open");

		is_open = true;
		return true;
	} while (0);
	
	aria2::libraryDeinit();
	return false;
}


shared_ptr<ptree> api_download::execute(const ptree& args, const api* /*caller*/) {
	shared_ptr<ptree> resp(new ptree());
	
	try {
		string opr = args.get<string>("opr");
		
		if (opr == "add") {
			ptree download_list = args.get_child("list");
			ptree ret_list;
			
			BOOST_FOREACH(const ptree::value_type& item, download_list) {
				vector<string> uris;
				BOOST_FOREACH(const ptree::value_type& uri, item.second.get_child("uris")) {
					uris.push_back(uri.second.get_value<string>());
				}
				
				aria2::KeyVals options;
				BOOST_FOREACH(const ptree::value_type& option, item.second.get_child("options")) {
					options.push_back(std::make_pair(option.second.get<string>("name"),
													 option.second.get<string>("value")));
				}
				
				string gid;
				int ret = add_uri(uris, options, 
								  item.second.get<int>("position"),
								  gid);
				
				ptree ret_pt;
				if (ret)
					ret_pt.put("result", "fail");
				else
					ret_pt.put("result", "success");
				ret_pt.put("gid", gid);
				ret_list.push_back(std::make_pair("", ret_pt));
			}
			
			resp->put("ret_code", 0);
			resp->add_child("list", ret_list);
			return resp;
			
		} else if (opr == "list") {
			vector<download_item> download_items;
			if (list_download_items(download_items)){
				resp->put("ret_code", 2);
				return resp;
			}
			
			ptree item_list;
			
			BOOST_FOREACH(download_item i, download_items) {
				item_list.push_back(std::make_pair("", *i.to_ptree()));
			}
			
			resp->put("ret_code", 0);
			resp->add_child("list", item_list);
			return resp;
			
		} else if (opr == "list_item") {
			string gid = args.get<string>("gid");
			
			aria2::DownloadHandle* dh = aria2::getDownloadHandle(aria2_session, aria2::hexToGid(gid));
			if (dh == NULL) {
				resp->put("ret_code", 1);
				return resp;
			}
			
			string name;
			if (dh->getNumFiles()) {
				string file_path = dh->getFile(1).path;
				string files_path = dh->getDir();
				name = file_path.substr(files_path.length() + 1);
				size_t found = name.rfind("/");
				if (found != std::string::npos)
					name = name.substr(0, found);
			}
			resp->put("name", name);
			
			resp->put("total_length", dh->getTotalLength());
			
			ptree option_list;
			aria2::KeyVals options = dh->getOptions();
			BOOST_FOREACH(const aria2::KeyVals::value_type& option, options) {
				ptree option_ptree;
				option_ptree.put("name", option.first);
				option_ptree.put("value", option.second);
				
				option_list.push_back(std::make_pair("", option_ptree));
			}
			resp->add_child("options", option_list);
			
			resp->put("ret_code", 0);
			return resp;
			
		}else if (opr == "remove") {
			ptree remove_list = args.get_child("list");
			ptree item_list;
			
			BOOST_FOREACH(const ptree::value_type& gid, remove_list) {
				ptree ret;
				if (remove_download(gid.second.get_value<string>()))
					ret.put("", "fail");
				else
					ret.put("", "success");
				item_list.push_back(std::make_pair("", ret));
			}
			
			resp->put("ret_code", 0);
			resp->add_child("list", item_list);
			return resp;
			
		} else if (opr == "global_option") {
			ptree option_list = args.get_child("list");
			ptree item_list;
			
			aria2::KeyVals options;
			BOOST_FOREACH(const ptree::value_type& option, option_list) {
				options.push_back(std::make_pair(option.second.get<string>("name"),
												 option.second.get<string>("value")));
			}
			
			if (change_global_option(options))          // 经测试不能以返回值判断是否设置成功
				resp->put("ret_code", 1);
			else
				resp->put("ret_code", 0);
			return resp;
			
		} else {
			debug_print("bad json!");
		}
		
	} catch (boost::property_tree::ptree_bad_path) {
		debug_print("bad json!");
	} catch (...) {
		debug_print("Unexpected error!");
	}
	
	resp->put("ret_code", 1);
	return resp;
}


void api_download::close() {
	if (!is_open)
		return;
	
	is_open = false;
	
	if (shutdown(aria2_session, true)) {                       // 需要强制退出，否则在下载bt资源时会不能及时退出
		BOOST_ASSERT(0 && "shutdown aria2 failed!");
	}
	
	aria2_thread->join();
	delete aria2_thread;
	aria2_thread = NULL;
	
	aria2::sessionFinal(aria2_session);
	aria2::libraryDeinit();
	aria2_session = NULL;
	
	debug_print("api_download close");
}


int api_download::add_uri(const vector<string>& uris, const aria2::KeyVals& options, int position,
						  string& gid) {
	BOOST_ASSERT(aria2_session != NULL && "aria2_session == NULL");
	
	aria2::A2Gid a2_gid;
	int ret = aria2::addUri(aria2_session, &a2_gid, uris, options, position);
	if (ret)
		return ret;
	
	gid = aria2::gidToHex(a2_gid);
	
	debug_print(gid + " added");
	return ret;
}


int api_download::remove_download(const string& gid) {
	BOOST_ASSERT(aria2_session != NULL && "aria2_session == NULL");
	
	int ret = aria2::removeDownload(aria2_session, aria2::hexToGid(gid));
	
	if (!ret)
		debug_print(gid + " deleted");
		
	return ret;
}


int api_download::list_download_items(vector<download_item>& download_items) {
	BOOST_ASSERT(aria2_session != NULL && "aria2_session == NULL");
	
	vector<aria2::A2Gid> gids = aria2::getActiveDownload(aria2_session);
	BOOST_FOREACH(aria2::A2Gid gid, gids) {
        aria2::DownloadHandle* dh = aria2::getDownloadHandle(aria2_session, gid);
		if(dh) {
			download_items.push_back(download_item(aria2::gidToHex(gid), dh));
			aria2::deleteDownloadHandle(dh);
		}
	}
	return 0;
}


int api_download::change_global_option(const aria2::KeyVals& options) {
	int ret = 0;
	
	if (!is_open) {
		BOOST_FOREACH(const aria2::KeyVals::value_type& option, options) {
			aria2_global_options.push_back(std::make_pair(option.first,
														  option.second));
		}
	} else {
		ret = aria2::changeGlobalOption(aria2_session, options);
	}
	
	if (!ret) {
		ptree info, poptions;
		
		BOOST_FOREACH(const aria2::KeyVals::value_type& option, options) {
			ptree item;
			
			item.put("name", option.first);
			item.put("value", option.second);
			
			poptions.push_back(std::make_pair("", item));
		}
		info.add_child("options", poptions);
		info.put("info", "global_options_changed");
		
		communicate(*this, info);
	} else
		debug_print("change options failed!");
		
	return ret;
}


void api_download::run_aria2(void* handle) {
	api_download* h = (api_download*)handle;
	
	aria2::run(h->aria2_session, aria2::RUN_DEFAULT);
	
	if (h->is_open) {
		BOOST_ASSERT(0 && "aria2 stopped unexpectedly!");
	}
	
	h->debug_print("api_download thread stopped");
}


int api_download::downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
									 aria2::A2Gid gid, void* user_data) {
	api_download* h = (api_download*)user_data;
	ptree event_ptree;
	
	switch(event) {
	case aria2::EVENT_ON_DOWNLOAD_START:
		event_ptree.put("event_type", "START");
		break;
	case aria2::EVENT_ON_DOWNLOAD_PAUSE:
		event_ptree.put("event_type", "PAUSE");
		break;
	case aria2::EVENT_ON_DOWNLOAD_STOP:
		event_ptree.put("event_type", "STOP");
		break;
	case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
		event_ptree.put("event_type", "COMPLETE");
		break;
	case aria2::EVENT_ON_DOWNLOAD_ERROR:
		event_ptree.put("event_type", "ERROR");
		break;
	default:
		return 0;
	}
	
	event_ptree.put("gid", aria2::gidToHex(gid));
	event_ptree.put("info", "event");
	
	h->communicate(*h, event_ptree);
	
	return 0;
}

// #define TEST
#ifdef TEST

#include <iostream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
using namespace std;

class api_download_listener : public api_listener {
public:
	virtual void debug(const boost::property_tree::ptree& content) {
		stringstream content_str;
		write_json(content_str, content);
		
		cout << "debug: " << content_str.str() << endl;
	}
	
	virtual boost::shared_ptr<boost::property_tree::ptree>
		communicate(const api& caller, const boost::property_tree::ptree& content) {
		stringstream content_str;
		write_json(content_str, content);
		
		cout << "communicate: " << content_str.str() << endl;
		
		return new ptree();
	}
};


int main() {
	api_download api;
	api_download_listener listener;
	char json_path[260];
	ptree json;
	
	if (api.open() == false) {
		cout << "open aria2 failed!" << endl;
		return 0;
	}
	
	api.attach_listener(&listener);
	
	while (true)
	{
		cout << "json file's path: ";
		cin.getline(json_path, 260);
		cout << json_path << endl;
		if (cin.eof())
			break;
		
		try {
			boost::property_tree::read_json(json_path, json);
		} catch (...) {
			cout << "parse json file failed!" << endl;
			continue;
		}
		
		shared_ptr<ptree> result = api.execute(json);
		
		std::stringstream result_str;
		write_json(result_str, *result);
		cout << "the result: " << endl
			 << result_str.str() << endl;
	}
	
	api.close();
	
	api.detach_listener(&listener);
	
	return 0;
}
#endif