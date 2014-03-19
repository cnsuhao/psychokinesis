#ifndef _API_DOWNLOAD_H_
#define _API_DOWNLOAD_H_

#include <vector>
#include <string>
#include <boost/assert.hpp>
#include <boost/thread/thread.hpp>
#include <aria2/aria2.h>
#include "api.h"

namespace psychokinesis {

struct download_item {
	download_item()
		: error_code(0), download_status(aria2::DOWNLOAD_ERROR),
		  total_length(0), completed_length(0), upload_length(0),
		  download_speed(0), upload_speed(0),
		  connections(0)
	{}
	
	download_item(const std::string& gid, aria2::DownloadHandle* dh)
		: gid(gid),
		  error_code(dh->getErrorCode()), download_status(dh->getStatus()),
		  total_length(dh->getTotalLength()), 
		  completed_length(dh->getCompletedLength()), upload_length(dh->getUploadLength()),
		  download_speed(dh->getDownloadSpeed()), upload_speed(dh->getUploadSpeed()),
		  connections(dh->getConnections())
	{}
	
	boost::shared_ptr<boost::property_tree::ptree> to_ptree() {
		boost::shared_ptr<boost::property_tree::ptree> ppt(new boost::property_tree::ptree());
		ppt->put("gid", gid);
		ppt->put("error_code", error_code);
		ppt->put("download_status", download_status);
		ppt->put("total_length", total_length);
		ppt->put("completed_length", completed_length);
		ppt->put("upload_length", upload_length);
		ppt->put("download_speed", download_speed);
		ppt->put("upload_speed", upload_speed);
		ppt->put("connections", connections);
		return ppt;
	}
	
	std::string gid;
	int error_code;
	aria2::DownloadStatus download_status;
	int64_t total_length;
	int64_t completed_length;
	int64_t upload_length;
	int download_speed;
	int upload_speed;
	int connections;
};

class api_download : public api {
public:
	api_download()
		: aria2_session(NULL), aria2_thread(NULL), is_open(false)
	{}
	
	virtual bool open();
	
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args);
	
	virtual void close();

private:
	int add_uri(const std::vector<std::string>& uris, const aria2::KeyVals& options, int position,
			   std::string& gid);
	
	int remove_download(const std::string& gid);
	
	int list_download_items(std::vector<download_item>& download_items);
	
	static void run_aria2(void* handle);
	
	static int downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
									 aria2::A2Gid gid, void* user_data);
	
	aria2::Session* aria2_session;
	boost::thread* aria2_thread;
	bool is_open;
};

} // namespace psychokinesis
#endif