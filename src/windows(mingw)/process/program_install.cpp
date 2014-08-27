#include <DuiLib/StdAfx.h>
#include <winreg.h>
#include "program_install.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <string>
#include <vector>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "config_control.h"
#include "../ui/encoding_changer.h"

#define APP_NAME "Psychokinesis"
#define PRODUCT_VERSION "1.0.0.1"
#define COMPANY_NAME "Psychokinesis Team"

using DuiLib::CStdString;
using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

static std::wstring ansi2unicode(const char* buf)
{
	int len = ::MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
    if (len == 0) return L"";

    std::vector<wchar_t> unicode(len);
    ::MultiByteToWideChar(CP_ACP, 0, buf, -1, &unicode[0], len);

    return &unicode[0];
}

program_install::install_result program_install::install() {
	CStdString exe_path = CPaintManagerUI::GetInstancePath();
	CStdString app_name = _T(APP_NAME);
	CStdString product_version = _T(PRODUCT_VERSION);
	CStdString company_name = _T(COMPANY_NAME);
	HKEY current_user_key = NULL, psychokinesis_key = NULL,
	uninstall_key = NULL, uninstall_psychokinesis_key = NULL;
	IShellLink *pshell_link = NULL, *pshell_link_uninstall = NULL;
	IPersistFile *ppersist_file = NULL, *ppersist_uninstall_file = NULL;
	install_result ret = failed;
	
	HRESULT hr_com = ::CoInitialize(NULL);
	
	do {
		// 在注册表HKEY_CURRENT_USER\Software下新建Psychokinesis项，用于存放路径值
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software"), 0, KEY_WRITE|KEY_READ,
						 &current_user_key) != ERROR_SUCCESS)
			break;
		
		DWORD has_key;
		if (RegCreateKeyEx(current_user_key, _T(APP_NAME), 0, REG_NONE, 
						   REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
						   &psychokinesis_key, &has_key) != ERROR_SUCCESS)
			break;
		
		if (has_key == REG_OPENED_EXISTING_KEY) {
			ret = installed;
			break;
		}
		
		if (RegSetValueEx(psychokinesis_key, _T("Path"), 0, REG_SZ, 
						  (const BYTE*)exe_path.GetData(), (exe_path.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;

		
		// 在注册表HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall下新建Psychokinesis项，用于存放卸载信息
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_WRITE|KEY_READ,
						 &uninstall_key) != ERROR_SUCCESS)
			break;
			
		if (RegCreateKeyEx(uninstall_key, _T(APP_NAME), 0, REG_NONE, 
						   REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
						   &uninstall_psychokinesis_key, &has_key) != ERROR_SUCCESS)
			break;
			
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("DisplayName"), 0, REG_SZ, 
						  (const BYTE*)app_name.GetData(), (app_name.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;
			
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("DisplayVersion"), 0, REG_SZ, 
						  (const BYTE*)product_version.GetData(), (product_version.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;
		
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("Publisher"), 0, REG_SZ, 
						  (const BYTE*)company_name.GetData(), (company_name.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;
			
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("InstallLocation"), 0, REG_SZ, 
						  (const BYTE*)exe_path.GetData(), (exe_path.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;
			
		TCHAR module_path[MAX_PATH];
		::GetModuleFileName(NULL, module_path, MAX_PATH);
		
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("DisplayIcon"), 0, REG_SZ, 
						  (const BYTE*)module_path, MAX_PATH)
			!= ERROR_SUCCESS)
			break;
			
		CStdString uninstall_string(module_path);
		uninstall_string += _T(" *u");
		if (RegSetValueEx(uninstall_psychokinesis_key, _T("UninstallString"), 0, REG_SZ, 
						  (const BYTE*)uninstall_string.GetData(), (uninstall_string.GetLength() + 1) * sizeof(TCHAR))
			!= ERROR_SUCCESS)
			break;

		
		// 初始化下载存储路径（AppData）
		char appdata_path[MAX_PATH];
		if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata_path)
				!= S_OK)
			break;
		std::string str_download_path = encoding_changer::ascii2utf8(appdata_path);
		str_download_path += "\\" APP_NAME;               // 程序启动配置导入时会确保路径存在
		
		try {
			boost::property_tree::ptree root, download_path;
			
			boost::property_tree::read_xml(CONFIG_FILE, root);
			
			boost::property_tree::ptree& download_config = root.get_child("config").get_child("download");
			download_path.put("name", "dir");
			download_path.put("value", str_download_path);
			download_config.push_back(std::make_pair("KeyVals", download_path));
			
			boost::property_tree::write_xml(CONFIG_FILE, root);
		} catch (...) {
			break;
		}
		
		
		// 创建快捷方式
		if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, 
							 CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pshell_link)))
			break;
		if (pshell_link->QueryInterface(IID_IPersistFile, (void**)&ppersist_file)
			!= S_OK)
			break;
		// 设置快捷方式的目标所在的位置(源程序完整路径) 
		pshell_link->SetPath(module_path);
		// 应用程序的工作目录 
		// 当用户没有指定一个具体的目录时，快捷方式的目标应用程序将使用该属性所指定的目录来装载或保存文件
		pshell_link->SetWorkingDirectory(exe_path.GetData());
		
		// （1）桌面快捷方式
		TCHAR desktop_path[MAX_PATH];
		if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path)
			!= S_OK)
			break;
		CStdString str_desktop_path = desktop_path;
		str_desktop_path += _T("\\" APP_NAME ".lnk");
		ppersist_file->Save(ansi2unicode(str_desktop_path.GetData()).c_str(), TRUE);

		// （2）开始菜单快捷方式
		TCHAR startmenu_path[MAX_PATH];
		if (SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startmenu_path)
			!= S_OK)
			break;
		CStdString str_startmenu_path = startmenu_path;
		str_startmenu_path += _T("\\" APP_NAME);
		boost::filesystem::path startmenu_dir(str_startmenu_path.GetData());
		boost::filesystem::create_directories(startmenu_dir);
		
		CStdString str_startmenu_startup_link = str_startmenu_path;
		str_startmenu_startup_link += _T("\\" APP_NAME ".lnk");
		ppersist_file->Save(ansi2unicode(str_startmenu_startup_link.GetData()).c_str(), TRUE);
		
		// （3）开始菜单卸载快捷方式
		if (!SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, 
							 CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pshell_link_uninstall)))
			break;
		if (pshell_link_uninstall->QueryInterface(IID_IPersistFile, (void**)&ppersist_uninstall_file)
			!= S_OK)
			break;
		// 设置快捷方式的目标所在的位置(源程序完整路径) 
		pshell_link_uninstall->SetPath(module_path);
		pshell_link_uninstall->SetArguments(_T("*u"));
		// 应用程序的工作目录 
		// 当用户没有指定一个具体的目录时，快捷方式的目标应用程序将使用该属性所指定的目录来装载或保存文件
		pshell_link_uninstall->SetWorkingDirectory(exe_path.GetData());
		
		CStdString str_startmenu_uninstall_link = str_startmenu_path;
		str_startmenu_uninstall_link += _T("\\卸载" APP_NAME ".lnk");
		ppersist_uninstall_file->Save(ansi2unicode(str_startmenu_uninstall_link.GetData()).c_str(), TRUE);
		
		ret = success;
	} while(0);
	
	// 回滚
	if (ret == failed)
		program_install::uninstall();
	
	if (ppersist_file) {
		ppersist_file->Release();
	}
	if (pshell_link) {
		pshell_link->Release();
	}
	if (ppersist_uninstall_file) {
		ppersist_uninstall_file->Release();
	}
	if (pshell_link_uninstall) {
		pshell_link_uninstall->Release();
	}
	if (current_user_key) {
		RegCloseKey(current_user_key);
		current_user_key = NULL;
	}
	if (psychokinesis_key) {
		RegCloseKey(psychokinesis_key);
		psychokinesis_key = NULL;
	}
	if (uninstall_key) {
		RegCloseKey(uninstall_key);
		uninstall_key = NULL;
	}
	if (uninstall_psychokinesis_key) {
		RegCloseKey(uninstall_psychokinesis_key);
		uninstall_psychokinesis_key = NULL;
	}
	if (!FAILED(hr_com)) {
		::CoUninitialize();
	}
	return ret;
}


void program_install::uninstall() {
	HKEY current_user_key = NULL, uninstall_key = NULL;
	
	do {
		// 删除注册表相关键值
		if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software"), 0, KEY_ALL_ACCESS,
						 &current_user_key) == ERROR_SUCCESS)
			RegDeleteKey(current_user_key, _T(APP_NAME));
		
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), 0, KEY_ALL_ACCESS,
						 &uninstall_key) == ERROR_SUCCESS)
			RegDeleteKey(uninstall_key, _T(APP_NAME));
			
		// 删除快捷方式
		TCHAR desktop_path[MAX_PATH];
		if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path)
			!= S_OK)
			break;
		CStdString str_desktop_path = desktop_path;
		str_desktop_path += _T("\\" APP_NAME ".lnk");
		DeleteFile(str_desktop_path.GetData());
		
		TCHAR startmenu_path[MAX_PATH];
		if (SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, startmenu_path)
			!= S_OK)
			break;
		CStdString str_startmenu_path = startmenu_path;
		str_startmenu_path += _T("\\" APP_NAME);
		boost::filesystem::remove_all(str_startmenu_path.GetData());
	} while (0);
	
	if (current_user_key) {
		RegCloseKey(current_user_key);
		current_user_key = NULL;
	}
	if (uninstall_key) {
		RegCloseKey(uninstall_key);
		uninstall_key = NULL;
	}
	
	// 删除应用程序
	char appdata_path[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata_path)
		!= S_OK)
		return;
	std::string str_bat_path = appdata_path;
	str_bat_path += "\\uninstall_psychokinesis.bat";
	char exe_path[MAX_PATH];
	GetModuleFileName(NULL, exe_path, MAX_PATH);
	std::string str_exe_path = exe_path;
	str_exe_path = str_exe_path.substr(0, str_exe_path.find_last_of("\\") + 1);
	
	std::ofstream bat_file(str_bat_path.c_str(), std::ofstream::out);
	bat_file << ":del" << std::endl
			 << " del \"" << str_exe_path << "psychokinesis.exe\"" << std::endl
			 << " del \"" << str_exe_path << "libUIlib.dll\"" << std::endl
			 << " del \"" << str_exe_path << "session.dat\"" << std::endl
			 << " del \"" << str_exe_path << "config.xml\"" << std::endl
			 << " del \"" << str_exe_path << "dht.dat\"" << std::endl
			 << " rd  /s/q \"" << str_exe_path << "res\"" << std::endl
			 << "if exist \"" << str_exe_path << "psychokinesis.exe\" goto del" << std::endl
			 << "rd /q \"" << str_exe_path << "\" 2>nul" << std::endl
			 << "del %0" << std::endl;
	bat_file.close();
	
	WinExec(str_bat_path.c_str(), 0);
}


// #define TEST
#ifdef TEST
#include <stdio.h>
#include <iostream>

using namespace std;

int main() {
	CoInitialize(NULL);
	
	cout << "testing install" << endl;
	program_install::install_result ret = program_install::install();
	cout << "install result: " << ret << endl;
	
	getchar();
	
	cout << "testing install" << endl;
	program_install::uninstall();
	
	CoUninitialize();
	return 0;
}

#endif
