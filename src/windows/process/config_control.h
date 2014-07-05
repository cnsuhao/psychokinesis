#ifndef _CONFIG_CONTROL_H_
#define _CONFIG_CONTROL_H_

#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/serialization/singleton.hpp>

#define CONFIG_FILE "config.xml"

namespace psychokinesis {

class config_control : public boost::serialization::singleton<config_control> {
public:
	bool load_config();
	void save_config();
	
	std::string account_get() {
		return account;
	}
	
	std::string password_get() {
		return password;
	}
	
private:
	void communication_load(const boost::property_tree::ptree& config);
	boost::shared_ptr<boost::property_tree::ptree> communication_save();
	void download_load(const boost::property_tree::ptree& config);
	boost::shared_ptr<boost::property_tree::ptree> download_save();
	
	std::string account;
	std::string password;
};

} // namespace psychokinesis

#endif