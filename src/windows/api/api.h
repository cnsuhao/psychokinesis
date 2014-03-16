#ifndef _API_H_
#define _API_H_

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

namespace psychokinesis {

class api {
public:
	virtual bool open() = 0;
	virtual boost::shared_ptr<boost::property_tree::ptree> execute(const boost::property_tree::ptree& args) = 0;
	virtual void close() = 0;
};

} // namespace psychokinesis
#endif