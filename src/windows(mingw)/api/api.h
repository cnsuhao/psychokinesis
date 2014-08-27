#ifndef _API_H_
#define _API_H_

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/ptr_container/ptr_container.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace psychokinesis {

class api;

class api_listener {
public:
	virtual void debug(const boost::property_tree::ptree& content) = 0;
	virtual boost::shared_ptr<boost::property_tree::ptree> 
		communicate(const api& caller, const boost::property_tree::ptree& content) = 0; 
};

class api {
public:
	virtual bool open() = 0;
	
	virtual boost::shared_ptr<boost::property_tree::ptree> 
		execute(const boost::property_tree::ptree& args, const api* caller = NULL) = 0;
	
	virtual void close() = 0;
	
	void attach_listener(api_listener* listener) {
		BOOST_ASSERT(listener && "listener == NULL!");
		
		listeners.push_back(listener);
	}
	
	void detach_listener(api_listener* listener) {
		BOOST_ASSERT(listener && "listener == NULL!");
		boost::unique_lock<boost::shared_mutex> lock(listeners_mutex);
		
		listeners.remove(listener);
	}
	
protected:
	void debug(const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		
		BOOST_FOREACH(api_listener* l, listeners) {
			l->debug(content);
		}
	}
	
	boost::shared_ptr< std::vector<std::string> > communicate(const api& caller, const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		boost::shared_ptr< std::vector<std::string> > resps(new std::vector<std::string>());
		
		BOOST_FOREACH(api_listener* l, listeners) {
			boost::shared_ptr<boost::property_tree::ptree> resp = l->communicate(caller, content);
			
			std::stringstream resp_sstr;
			if (!resp->empty()) {
				boost::property_tree::write_json(resp_sstr, *resp);     // 相应数据格式在这儿指定，独立于具体的API
				resps->push_back(resp_sstr.str());
			}
		}
		
		return resps;
	}
	
	void debug_print(const std::string& d) {
#ifdef PSYCHOKINESIS_DEBUG
		boost::property_tree::ptree debug_ptree;
		debug_ptree.put("debug", d);
		debug(debug_ptree);
#endif
	}
	
private:
	std::list<api_listener*> listeners;
	boost::shared_mutex listeners_mutex;
};

} // namespace psychokinesis
#endif