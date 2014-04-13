#include <DuiLib/StdAfx.h>
#include "frame_window.h"
#include "../process/control.h"

using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

void window_message_loop();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
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
			MessageBox(NULL, _T("程序初始化失败!"), _T("错误"), MB_ICONERROR | MB_OK);
			break;
		}
	
		// 初始化界面
		frame_window& m_window = frame_window::get_mutable_instance();
		if (!m_window.create()) {
			MessageBox(NULL, _T("界面初始化失败!"), _T("错误"), MB_ICONERROR | MB_OK);
			m_control.close();
			break;
		}
	
		window_message_loop();
	
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
