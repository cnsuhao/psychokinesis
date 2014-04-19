#pragma once

#include <string>


namespace psychokinesis {

class api_message {
public:
	virtual void execute() = 0;             // 界面线程调用
};

class api_communication_logging : public api_message {
public:
	api_communication_logging(const std::string& account, const std::string& password)
		: account(account), password(password)
	{}
	
	virtual void execute();
	
private:
	std::string account;
	std::string password;
};

class api_communication_logged : public api_message {
public:
	api_communication_logged(const std::string& account, const std::string& password)
		: account(account), password(password)
	{}
	
	virtual void execute();
	
private:
	std::string account;
	std::string password;
};

class api_communication_login_failed : public api_message {
public:
	api_communication_login_failed(int error_code)
		: error_code(error_code)
	{}
	
	virtual void execute();
	
private:
	int error_code;
};

class api_download_store_path_changed : public api_message {
public:
	api_download_store_path_changed(const std::string& new_path)
		: new_path(new_path)
	{}
	
	virtual void execute();
	
private:
	std::string new_path;
};

} // namespace psychokinesis