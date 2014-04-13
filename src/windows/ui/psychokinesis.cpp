#include <DuiLib/StdAfx.h>
#include "frame_window.h"

using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

void window_message_loop();

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("res"));
	
	HRESULT Hr = ::CoInitialize(NULL);                // 仅在界面线程
	if(FAILED(Hr)) {
		MessageBox(NULL, _T("初始化COM失败!"), _T("错误"), MB_ICONERROR | MB_OK);
		return 1;
	}
	
	frame_window& window = frame_window::get_mutable_instance();
	if (!window.create())
		return 2;
	
	window_message_loop();
	
	window.destory();
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
