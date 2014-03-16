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
		
		aria2_session = aria2::sessionNew(aria2::KeyVals(), config);
		if (aria2_session == NULL)
			break;
			
		aria2_thread = new thread(run_aria2, this);
		if (aria2_thread == NULL) {
			aria2::sessionFinal(aria2_session);
			aria2_session = NULL;
			break;
		}
		
		is_open = true;
		return true;
	} while (0);
	
	aria2::libraryDeinit();
	return false;
}


shared_ptr<ptree> api_download::execute(const ptree& args) {
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
				ptree it, options_ptree;
				
				it.put("gid", i.gid);
				it.put("error_code", i.error_code);
				it.put("download_status", i.download_status);
				it.put("total_length", i.total_length);
				it.put("completed_length", i.completed_length);
				it.put("upload_length", i.upload_length);
				it.put("download_speed", i.download_speed);
				it.put("upload_speed", i.upload_speed);
				it.put("connections", i.connections);
				
				for (aria2::KeyVals::iterator option = i.options.begin(); 
					 option != i.options.end(); ++option) {
					ptree option_ptree;
					option_ptree.put("name", option->first);
					option_ptree.put("value", option->second);
					options_ptree.push_back(std::make_pair("", option_ptree));
				}
				it.add_child("options", options_ptree);
				
				item_list.push_back(std::make_pair("", it));
			}
			
			resp->put("ret_code", 0);
			resp->add_child("list", item_list);
			return resp;
			
		} else if (opr == "remove") {
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
			
		} else {
			BOOST_ASSERT(0 && "bad json!");
		}
		
	} catch (boost::property_tree::ptree_bad_path) {
		BOOST_ASSERT(0 && "bad json!");
	}
	
	resp->put("ret_code", 1);
	return resp;
}


void api_download::close() {
	BOOST_ASSERT(is_open == true && "aria2 have closed!");
	
	is_open = false;
	
	if (shutdown(aria2_session)) {
		BOOST_ASSERT(0 && "shutdown aria2 failed!");
	}
	
	aria2_thread->join();
		
	aria2::sessionFinal(aria2_session);
	aria2_session = NULL;
	
	aria2::libraryDeinit();
}


int api_download::add_uri(const vector<string>& uris, const aria2::KeyVals& options, int position,
						  string& gid) {
	BOOST_ASSERT(aria2_session != NULL && "aria2_session == NULL");
	
	aria2::A2Gid a2_gid;
	int ret = aria2::addUri(aria2_session, &a2_gid, uris, options, position);
	if (ret)
		return ret;
	
	gid = aria2::gidToHex(a2_gid);
	return ret;
}


int api_download::remove_download(const string& gid) {
	BOOST_ASSERT(aria2_session != NULL && "aria2_session == NULL");
	
	return removeDownload(aria2_session, aria2::hexToGid(gid));
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


void api_download::run_aria2(void* handle) {
	api_download* h = (api_download*)handle;
	
	aria2::run(h->aria2_session, aria2::RUN_DEFAULT);
	
	if (h->is_open) {
		BOOST_ASSERT(0 && "aria2 stopped unexpectedly!");
	}
}

#define TEST
#ifdef TEST

#include <iostream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
using namespace std;

int main() {
	api_download api;
	char json_path[260];
	ptree json;
	
	if (api.open() == false) {
		cout << "open aria2 failed!" << endl;
		return 0;
	}
	
	while (true)
	{
		cin.getline(json_path, 260);
		cout << "json file's path: " << json_path << endl;
		
		try {
			boost::property_tree::read_json(json_path, json);
		} catch (...) {
			cout << "parse json file failed!" << endl;
			return 0;
		}
		
		boost::shared_ptr<ptree> result = api.execute(json);
		
		std::stringstream result_str;
		write_json(result_str, *result);
		cout << "the result: " << endl
			 << result_str.str() << endl;
	}
	
	api.close();
	return 0;
}
#endif