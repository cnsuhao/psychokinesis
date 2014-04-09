#pragma once

namespace psychokinesis {

class api_message {
public:
	virtual void execute() = 0;
};

class api_communication_logged : public api_message {
public:
	virtual void execute() {
	}
};

} // namespace psychokinesis