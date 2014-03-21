#ifndef _API_H_
#define _API_H_

#include <list>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/ptr_container/ptr_container.hpp>

namespace psychokinesis {

class api_listener {
public:
	virtual void debug(const boost::property_tree::ptree& content) = 0;
	virtual void info(const boost::property_tree::ptree& content) = 0;
	virtual void warning(const boost::property_tree::ptree& content) = 0;
	virtual void alert(const boost::property_tree::ptree& content) = 0;
	
	virtual void communicate(boost::property_tree::ptree& content) = 0; 
};

class api {
public:
	virtual bool open() = 0;
	
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args) = 0;
	
	virtual void close() = 0;
	
	// before open
	void attach_listener(api_listener* listener) {
		BOOST_ASSERT(listener && "listener == NULL!");
		
		listeners.push_back(listener);
	}
	
	// after close
	void detach_listener(api_listener* listener) {
		BOOST_ASSERT(listener && "listener == NULL!");
		
		listeners.remove(listener);
	}
	
protected:
	void debug(const boost::property_tree::ptree& content) {
		BOOST_FOREACH(api_listener* l, listeners) {
			l->debug(content);
		}
	}
	
	void info(const boost::property_tree::ptree& content) {
		BOOST_FOREACH(api_listener* l, listeners) {
			l->info(content);
		}
	}
	
	void warning(const boost::property_tree::ptree& content) {
		BOOST_FOREACH(api_listener* l, listeners) {
			l->warning(content);
		}
	}
	
	void alert(const boost::property_tree::ptree& content) {
		BOOST_FOREACH(api_listener* l, listeners) {
			l->alert(content);
		}
	}
	
	void communicate(boost::property_tree::ptree& content) {
		BOOST_FOREACH(api_listener* l, listeners) {
			l->communicate(content);
		}
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
};

} // namespace psychokinesis
#endif