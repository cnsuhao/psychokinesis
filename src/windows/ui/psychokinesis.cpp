#include <DuiLib/StdAfx.h>
#include "frame_window.h"
#include "../process/control.h"
#include "../process/program_install.h"

#define WM_UNINSTALL 0x0401                   // 卸载消息

using DuiLib::CStdString;
using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

// 定义一个唯一的窗口属性名，用于实现单实例程序
TCHAR g_window_prop_name[] = _T("57d0f19d-74c5-4ded-b6ad-0b085fe70b0f");
HANDLE g_window_value = (HANDLE)1022;

UINT window_message_loop();
BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR lpCmdLine, int nCmdShow){
	CStdString cmd_line = lpCmdLine;
	bool is_uninstall = cmd_line.Find(_T("*u")) >= 0;
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("res"));
	
	// 检测单实例程序
	HWND old_hwnd = NULL;
	::EnumWindows(enum_wnd_proc,(LPARAM)&old_hwnd);    // 枚举所有运行的窗口
	if (old_hwnd != NULL) {
		if (!is_uninstall) {
			::MessageBox(NULL, _T("程序已在运行!"), _T("错误"), MB_ICONERROR | MB_OK);
			return 0;
		} else {
			// 卸载程序
			if (::MessageBox(NULL, _T("确实要卸载Psychokinesis吗？"), _T("警告"), MB_ICONWARNING | MB_YESNO) == IDYES) {
				::PostMessage(old_hwnd, WM_UNINSTALL, 0, 0);
			}
			return 0;
		}
	}
	
	// 卸载程序
	if (is_uninstall) {
		if (::MessageBox(NULL, _T("确实要卸载Psychokinesis吗？"), _T("警告"), MB_ICONWARNING | MB_YESNO) == IDYES) {
			program_install::uninstall();
		}
		return 0;
	}
	
	// 安装程序
	if (cmd_line.Find(_T("*s")) >= 0) {
		program_install::install_result ret = program_install::install();
		if (ret == program_install::failed) {
			::MessageBox(NULL, _T("程序安装失败!请尝试以管理员权限重新运行安装程序。"), _T("错误"), MB_ICONERROR | MB_OK);
			return 0;
		}
	}
	
	// 初始化COM
	HRESULT Hr = ::CoInitialize(NULL);                // 仅在界面线程
	if (FAILED(Hr)) {
		MessageBox(NULL, _T("初始化COM失败!"), _T("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}
	
	UINT quit_msg;
	do {
		// 初始化后台
		control& m_control = control::get_mutable_instance();
		if (!m_control.open()) {
			MessageBox(NULL, _T("程序初始化失败!请尝试重新下载安装软件。"), _T("错误"), MB_ICONERROR | MB_OK);
			break;
		}
	
		// 初始化界面
		frame_window& m_window = frame_window::get_mutable_instance();
		if (!m_window.create()) {
			MessageBox(NULL, _T("界面初始化失败!请尝试重新下载安装软件。"), _T("错误"), MB_ICONERROR | MB_OK);
			m_control.close();
			break;
		}
		
		// 主窗口创建后设置，为窗口附加一个属性，用于实现单实例程序
		::SetProp(m_window.GetHWND(), g_window_prop_name, g_window_value);
		
		quit_msg = window_message_loop();
	
		// 主窗口退出时移除该附加属性，用于实现单实例程序
		::RemoveProp(m_window.GetHWND(), g_window_prop_name);
		
		m_window.destory();
		m_control.close();
	} while (0);
	
	::CoUninitialize();
	
	if (quit_msg == WM_UNINSTALL) {
		program_install::uninstall();
	}
	return 0;
}


UINT window_message_loop() {
	frame_window& window = frame_window::get_mutable_instance();
	MSG msg = {0};

	for (;;) {
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!CPaintManagerUI::TranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT || msg.message == WM_UNINSTALL)
				return msg.message;
		}

		if (!window.handle_background_message())
			::WaitMessage();
	}
	
	return 0;
}

BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam) {
	HANDLE h = ::GetProp(hwnd, g_window_prop_name);
	if(h == g_window_value)
	{
		*(HWND*)lparam = hwnd;
		return FALSE;
	}
	return TRUE;
}
