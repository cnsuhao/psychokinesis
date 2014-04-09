#include <DuiLib/StdAfx.h>
#include "frame_window.h"

using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("res"));
	
	HRESULT Hr = ::CoInitialize(NULL);
	if(FAILED(Hr)) {
		MessageBox(NULL, _T("初始化COM失败!"), _T("错误"), MB_ICONERROR | MB_OK);
		return 1;
	}
	
	frame_window window;
	if (!window.create())
		return 2;
	
	MSG msg = {0};
	while(::GetMessage(&msg, NULL, 0, 0)) {
		if(!CPaintManagerUI::TranslateMessage(&msg)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		} else {
			window.handle_background_message();
		}
	}
	
	::CoUninitialize();
	
	return 0;
}


//	int window_width = 460;
//	int window_height = 510;

