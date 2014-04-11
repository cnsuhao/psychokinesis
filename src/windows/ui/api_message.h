#pragma once

namespace psychokinesis {

class api_message {
public:
	virtual void execute() = 0;             // 界面线程调用
};

class api_communication_logged : public api_message {
public:
	virtual void execute();
};

class api_communication_login_failed : public api_message {
public:
	virtual void execute();
};

} // namespace psychokinesis