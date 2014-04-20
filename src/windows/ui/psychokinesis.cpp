#include <DuiLib/StdAfx.h>
#include "frame_window.h"
#include "../process/control.h"

using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

// 定义一个唯一的窗口属性名，用于实现单实例程序
TCHAR g_window_prop_name[] = _T("57d0f19d-74c5-4ded-b6ad-0b085fe70b0f");
HANDLE g_window_value = (HANDLE)1022;

void window_message_loop();
BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
	// 检测单实例程序
	HWND old_hwnd = NULL;
	::EnumWindows(enum_wnd_proc,(LPARAM)&old_hwnd);    // 枚举所有运行的窗口
	if (old_hwnd != NULL) {
		::MessageBox(NULL, _T("程序已在运行!"), _T("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}
	
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("res"));
	
	// 初始化COM
	HRESULT Hr = ::CoInitialize(NULL);                // 仅在界面线程
	if (FAILED(Hr)) {
		MessageBox(NULL, _T("初始化COM失败!"), _T("错误"), MB_ICONERROR | MB_OK);
		return 0;
	}
	
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
		
		window_message_loop();
	
		// 主窗口退出时移除该附加属性，用于实现单实例程序
		::RemoveProp(m_window.GetHWND(), g_window_prop_name);
		
		m_window.destory();
		m_control.close();
	} while (0);
	
	::CoUninitialize();
	
	return 0;
}


void window_message_loop() {
	frame_window& window = frame_window::get_mutable_instance();
	MSG msg = {0};

	for (;;) {
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!CPaintManagerUI::TranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
				return;
		}

		if (!window.handle_background_message())
			::WaitMessage();
	}
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
