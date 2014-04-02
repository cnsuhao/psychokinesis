#include <UIlib/StdAfx.h>

class frame_window : public CWindowWnd, public INotifyUI {
public:
	frame_window() {
		position.left = position.top = position.right = position.bottom = 0;
	}
	
	void create();
	
	void on_destroy();
	
	// 获取窗口类名接口
	LPCTSTR GetWindowClassName() const {
		return _T("frame_window");
	}
	
	// 获取窗口风格接口
	UINT GetClassStyle() const {
		return UI_CLASSSTYLE_FRAME;
	}
	
	void Notify(TNotifyUI& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	
private:
	RECT position;
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
	CPaintManagerUI::SetResourceInstance(hInstance);
	
	HRESULT Hr = ::CoInitialize(NULL);
	if(FAILED(Hr))
		return 0;
	
	if(::LoadLibrary("d3d9.dll") == NULL)
		return 0;
	
	frame_window window;
	window.create();
	
	CPaintManagerUI::MessageLoop();
	
	::CoUninitialize();
	
	return 0;
}



void frame_window::create() {
	int window_width = 460;
	int window_height = 510;
	int screen_width = ::GetSystemMetrics(SM_CXSCREEN);
	int screen_height = ::GetSystemMetrics(SM_CYSCREEN);
	
	// 窗口居中
	position.left = (screen_width - window_width) / 2;
	position.top = (screen_height - window_height) / 2;
	position.right = position.left + window_width;
	position.bottom = position.top + window_height;
	
	CWindowWnd::Create(NULL, 
					   "Psychokinesis", 
					   UI_WNDSTYLE_FRAME & 
					   ~WS_MAXIMIZEBOX &          // 无最大化按钮
					   ~WS_MINIMIZEBOX &          // 无最小化按钮
					   ~WS_SIZEBOX,               // 不可调整大小
					   WS_EX_WINDOWEDGE,
					   position);
}


void frame_window::on_destroy() {
	::PostQuitMessage(0L);
}


void frame_window::Notify(TNotifyUI& msg) {
}


LRESULT frame_window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE:
			break;
		case WM_DESTROY:
			on_destroy();
			break;
		case WM_ERASEBKGND:
			return 1;
		case WM_PAINT:
			return 0;
		default:
			break;
	}
	
	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
