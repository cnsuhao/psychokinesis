#include <windows.h>
#include <vector>
#include "encoding_changer.h"

using std::string;
using std::vector;
using namespace psychokinesis;


string encoding_changer::ascii2utf8(const string& str) {
	const char* buf = str.c_str();
	
	int nlen = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	if (nlen == 0)
		return "";
	std::vector<wchar_t> unicode(nlen);
	::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], nlen);

	nlen = ::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nlen);
	::WideCharToMultiByte(CP_UTF8, 0, &unicode[0], -1, &utf8[0], nlen, NULL, NULL);
	return &utf8[0];
}

string encoding_changer::utf82ascii(const string& str) {
	const char* buf = str.c_str();
	
	int   nLen = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	if (nLen == 0)
		return "";
	std::vector<wchar_t> uBuf(nLen);
	::MultiByteToWideChar(CP_UTF8, 0, buf, -1, &uBuf[0], nLen);

	nLen = ::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, NULL, 0, NULL, NULL);
	std::vector<char> utf8(nLen);
	::WideCharToMultiByte(CP_ACP, 0, &uBuf[0], -1, &utf8[0], nLen, NULL, NULL);
	return &utf8[0];
}
