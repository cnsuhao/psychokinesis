#ifndef _PROGRAM_INSTALL_H_
#define _PROGRAM_INSTALL_H_

namespace psychokinesis {

class program_install {
public:
	enum install_result {
		failed,
		installed,
		success
	};
	
	static install_result install();
	static void uninstall();
};

} // namespace psychokinesis

#endif