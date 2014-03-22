#ifndef _API_H_
#define _API_H_

#include <list>
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

class api_listener {
public:
	virtual void debug(const boost::property_tree::ptree& content) = 0;
	virtual void warning(const boost::property_tree::ptree& content) = 0;
	virtual void alert(const boost::property_tree::ptree& content) = 0;
	
	virtual void info(const boost::property_tree::ptree& content) = 0;
	virtual void communicate(boost::property_tree::ptree& content) = 0; 
};

class api {
public:
	virtual bool open() = 0;
	
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args) = 0;
	
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
	
	void info(const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		
		BOOST_FOREACH(api_listener* l, listeners) {
			l->info(content);
		}
	}
	
	void warning(const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		
		BOOST_FOREACH(api_listener* l, listeners) {
			l->warning(content);
		}
	}
	
	void alert(const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		
		BOOST_FOREACH(api_listener* l, listeners) {
			l->alert(content);
		}
	}
	
	boost::shared_ptr<std::string> communicate(const boost::property_tree::ptree& content) {
		boost::shared_lock<boost::shared_mutex> lock(listeners_mutex);
		boost::property_tree::ptree resp = content;
		
		BOOST_FOREACH(api_listener* l, listeners) {
			l->communicate(resp);
		}
		
		std::stringstream resp_sstr;
		boost::property_tree::write_json(resp_sstr, resp);     // 相应数据格式在这儿指定，独立于具体的API
		
		boost::shared_ptr<std::string> resp_str(new std::string(resp_sstr.str()));
		return resp_str;
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