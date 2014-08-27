#pragma once

#include <string>

namespace psychokinesis {

class encoding_changer {
public:
	static std::string ascii2utf8(const std::string& str);
	static std::string utf82ascii(const std::string& str);
};

} // namespace psychokinesis