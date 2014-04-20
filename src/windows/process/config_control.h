#ifndef _CONFIG_CONTROL_H_
#define _CONFIG_CONTROL_H_

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

namespace psychokinesis {

class config_control {
public:
	bool load_config();
	void save_config();
	
private:
	void communication_load(const boost::property_tree::ptree& config);
	boost::shared_ptr<boost::property_tree::ptree> communication_save();
	void download_load(const boost::property_tree::ptree& config);
	boost::shared_ptr<boost::property_tree::ptree> download_save();
};

} // namespace psychokinesis

#endif