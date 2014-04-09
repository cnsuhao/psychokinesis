#include <boost/assign.hpp>
#include "frame_window.h"
#include "resource.h"

#define CONTROL_NAME_TAB              "switch"

using DuiLib::CSliderUI;
using DuiLib::TNotifyUI;
using DuiLib::CStdString;
using DuiLib::CTabLayoutUI;
using DuiLib::CDialogBuilder;
using DuiLib::CControlUI;

using namespace psychokinesis;

frame_window::frame_window() {
	boost::assign::insert(tabs)
		("account", 0)
		("bandwidth", 1);
}


void frame_window::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("windowinit")) {
		make_notify(_T("account"), &frame_window::on_tab_notify);
		make_notify(_T("bandwidth"), &frame_window::on_tab_notify);
		make_notify(_T("loginbtn"), &frame_window::on_login_notify);
	}
}


LRESULT frame_window::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT ret = 0;
	bool handled = true;

	switch (uMsg) {
	case WM_CREATE:
		ret = on_create(wParam, lParam, handled);
		break;
	case WM_CLOSE:
		ret = on_close(wParam, lParam, handled);
		break;
	case WM_DESTROY:
		ret = on_destory(wParam, lParam, handled);
		break;
	case WM_SYSCOMMAND:
		ret = on_syscommand(wParam, lParam, handled);
		break;
	default:
		handled = false;
		break;
	}

	if(handled) 
		return ret;

	if(m_pm.MessageHandler(uMsg, wParam, lParam, ret)) 
			return ret;

	return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}


bool frame_window::create() {
	if (!this->Create(NULL, 
					  _T("Psychokinesis"),
					  UI_WNDSTYLE_FRAME & 
					  ~WS_MAXIMIZEBOX &          // 无最大化按钮
					  ~WS_MINIMIZEBOX &          // 无最小化按钮
					  ~WS_SIZEBOX,               // 不可调整大小
					  0L, 0, 0, 
					  460, 510))
		return false;

	this->CenterWindow();
	::ShowWindow(*this, SW_SHOW);

	return true;
}


void frame_window::handle_background_message() {
	if (api_msgs.size() == 0)
		return;

	boost::ptr_deque<api_message>::auto_type msg = api_msgs.pop_front();
	msg->execute();
}


LRESULT frame_window::on_create(WPARAM wParam, LPARAM lParam, bool& handled) {
	CDialogBuilder builder;

	::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1));

	m_pm.Init(m_hWnd);
	CControlUI* root = builder.Create(_T("frame.xml"), (UINT)0,  NULL, &m_pm);
	if (!root){
		MessageBox(NULL, _T("找不到所需的资源文件！请尝试重新下载安装软件。"), _T("错误"), MB_ICONERROR | MB_OK);
		::PostQuitMessage(0L);
		return 0;
	}

	m_pm.AttachDialog(root);
	m_pm.AddNotifier(this);

	return 0;
}


LRESULT frame_window::on_close(WPARAM wParam, LPARAM lParam, bool& handled) {
	handled = false;
	return 0;
}


LRESULT frame_window::on_destory(WPARAM wParam, LPARAM lParam, bool& handled) {
	::PostQuitMessage(0L);

	handled = false;
	return 0;
}


LRESULT frame_window::on_syscommand(WPARAM wParam, LPARAM lParam, bool& handled) {
	if(wParam == SC_CLOSE) {
		::PostQuitMessage(0L);
		handled = true;
		return 0;
	}

	return CWindowWnd::HandleMessage(WM_SYSCOMMAND, wParam, lParam);
}


bool frame_window::on_tab_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("click")) {
		CStdString name = pmsg->pSender->GetName();
		CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_pm.FindControl(_T(CONTROL_NAME_TAB)));

		boost::unordered_map<std::string, int>::const_iterator it = tabs.find(name.GetData());
		if (it != tabs.end()) {
			pControl->SelectItem(it->second);
		}
	}

	return true;
}


bool frame_window::on_login_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("click")) {
	}

	return true;
}