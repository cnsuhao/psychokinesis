#include <boost/assign.hpp>
#include "frame_window.h"
#include "resource.h"
#include "api_message.h"
#include <shlobj.h>

using DuiLib::CSliderUI;
using DuiLib::TNotifyUI;
using DuiLib::CStdString;
using DuiLib::CTabLayoutUI;
using DuiLib::CDialogBuilder;
using DuiLib::CControlUI;
using DuiLib::CEditUI;
using DuiLib::CButtonUI;
using DuiLib::COptionUI;

using namespace psychokinesis;

frame_window::frame_window() {
	boost::assign::insert(tabs)
		("account", 0)
		("bandwidth", 1);

	// 获取Windows版本号
	::ZeroMemory(&windows_version, sizeof(OSVERSIONINFO));
    windows_version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&windows_version);

	// 获取程序图标，无需手动释放，随程序自动释放
	m_icon = ::LoadIcon(GetModuleHandle(NULL), (LPCTSTR)IDI_ICON1);
}


void frame_window::Notify(TNotifyUI& msg) {
	if (msg.sType == _T("windowinit")) {
		on_windowinit_notify(msg);
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
	case WM_COMMAND:
		ret = on_command(wParam, lParam, handled);
		break;
	case 0x0401:
		ret = on_tray(wParam, lParam, handled);
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

	if (!add_tray())
		return false;

	this->CenterWindow();
	::ShowWindow(*this, SW_SHOW);

	return true;
}


bool frame_window::handle_background_message() {
	if (api_msgs.size() == 0)
		return false;

	boost::ptr_deque<api_message>::auto_type msg = api_msgs.pop_front();
	msg->execute();
	return true;
}


void frame_window::destory() {
	delete_tray();
}


bool frame_window::add_tray() {
	m_tray.cbSize = sizeof(NOTIFYICONDATA);
	m_tray.hWnd = m_hWnd;
	m_tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_tray.uCallbackMessage = 0x0401;
	strncpy(m_tray.szTip, "Psychokinesis", 128);
	m_tray.hIcon = m_icon;
	return ::Shell_NotifyIcon(NIM_ADD, &m_tray) == 1;
}


void frame_window::delete_tray() {
	::Shell_NotifyIcon(NIM_DELETE, &m_tray);
}


void frame_window::popup_tray_menu() {
	HMENU hmenu;            // menu template          
	HMENU hmenuTrackPopup;  // shortcut menu   
	POINT mouse_point = {0, 0};
	::GetCursorPos(&mouse_point);

	//  Load the menu template containing the shortcut menu from the 
	//  application's resources. 
	hmenu = ::LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_TRAY_MENU)); 
	if (hmenu == NULL) 
		return; 

	// Get the first shortcut menu in the menu template. This is the 
	// menu that TrackPopupMenu displays. 
	hmenuTrackPopup = GetSubMenu(hmenu, 0);

	// Draw and track the shortcut menu.
	TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
		mouse_point.x, mouse_point.y, 0, m_hWnd, NULL); 

	// Destroy the menu.
	DestroyMenu(hmenu);
}


LRESULT frame_window::on_create(WPARAM wParam, LPARAM lParam, bool& handled) {
	CDialogBuilder builder;

	::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)m_icon);

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
		::ShowWindow(*this, SW_HIDE);
		handled = true;
		return 0;
	}

	return CWindowWnd::HandleMessage(WM_SYSCOMMAND, wParam, lParam);
}


LRESULT frame_window::on_command(WPARAM wParam, LPARAM lParam, bool& handled) {
	switch(LOWORD(wParam)) {
	case ID_TRAY_CONFIG:
		::ShowWindow(*this, SW_SHOW);
		break;
	case ID_TRAY_ABOUT:
		break;
	case ID_TRAY_EXIT:
		::PostQuitMessage(0L);
		handled = true;
		return 0;
	default:
		break;
	}

	return CWindowWnd::HandleMessage(WM_COMMAND, wParam, lParam);
}


LRESULT frame_window::on_tray(WPARAM wParam, LPARAM lParam, bool& handled) {
	if (lParam == WM_RBUTTONUP) {
		popup_tray_menu();
	} else if (lParam == WM_LBUTTONDBLCLK) {
		if (!::IsWindowVisible(*this))
			::ShowWindow(*this, SW_SHOW);
		else
			::ShowWindow(*this, SW_HIDE);
	} else if (lParam == WM_LBUTTONUP) {
		if (::IsWindowVisible(*this)) {
			// 使窗口最前
			::SetWindowPos(*this, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			::SetWindowPos(*this, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}

	return  CWindowWnd::HandleMessage(0x0401, wParam, lParam);
}


void frame_window::on_windowinit_notify(DuiLib::TNotifyUI& msg) {
	make_notify(_T("account"), &frame_window::on_tab_notify);
	make_notify(_T("bandwidth"), &frame_window::on_tab_notify);
	make_notify(_T("loginbtn"), &frame_window::on_login_notify);
	make_notify(_T("registerbtn"), &frame_window::on_register_notify);
	make_notify(_T("changestorepathbtn"), &frame_window::on_changestorepath_notify);
	make_notify(_T("nolimitdownload"), &frame_window::on_nolimitdownload_notify);
	make_notify(_T("limitdownload"), &frame_window::on_limitdownload_notify);
	make_notify(_T("nolimitupload"), &frame_window::on_nolimitupload_notify);
	make_notify(_T("limitupload"), &frame_window::on_limitupload_notify);

	CEditUI* changestorepath_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("changestorepathedit")));
	changestorepath_edit->SetEnabled(false);

	//TODO 根据用户配置设置
	CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
	download_edit->SetEnabled(false);
	CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
	upload_edit->SetEnabled(false);
}


bool frame_window::on_tab_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CStdString name = pmsg->pSender->GetName();
		CTabLayoutUI* pControl = dynamic_cast<CTabLayoutUI*>(m_pm.FindControl(_T("switch")));

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
		CEditUI* account_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
		CEditUI* password_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
		CStdString account = account_edit->GetText();
		CStdString password = password_edit->GetText();

		if (account.GetLength() > 0 && password.GetLength() > 0) {
			account_edit->SetEnabled(false);
			password_edit->SetEnabled(false);

			CButtonUI* login_button = dynamic_cast<CButtonUI*>(pmsg->pSender);
			login_button->SetEnabled(false);

			// 测试代码
			api_msgs.push_back(new api_communication_login_failed());
		}
	}

	return true;
}


bool frame_window::on_register_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("click")) {
		::ShellExecute(m_hWnd, _T("open"), _T("http://www.baidu.com"), NULL, NULL, SW_SHOW);
	}
	return true;
}


bool frame_window::on_changestorepath_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("click")) {
		CStdString promot = _T("为您使用psychokinesis管理的资源选择位置");

		::BROWSEINFO bi;
		memset(&bi, 0, sizeof(bi));

		bi.hwndOwner = NULL;
		bi.lpszTitle = promot;
		bi.ulFlags   = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS| BIF_NEWDIALOGSTYLE;

		LPITEMIDLIST pidl = ::SHBrowseForFolder(&bi);
		if (pidl == NULL) {
			return true;
		}

		TCHAR buffer[MAX_PATH];
		if(!::SHGetPathFromIDList(pidl, buffer) != 0) {
			::CoTaskMemFree(pidl);
			return true;
		}

		CEditUI* changestorepath_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("changestorepathedit")));
		changestorepath_edit->SetText(buffer);

		::CoTaskMemFree(pidl);
	}
	return true;
}


bool frame_window::on_nolimitdownload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
		download_edit->SetEnabled(false);
	}

	return true;
}


bool frame_window::on_limitdownload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
		download_edit->SetEnabled(true);
	}

	return true;
}


bool frame_window::on_nolimitupload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
		upload_edit->SetEnabled(false);
	}

	return true;
}


bool frame_window::on_limitupload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
		upload_edit->SetEnabled(true);
	}

	return true;
}
