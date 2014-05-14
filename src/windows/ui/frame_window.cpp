#include <boost/assign.hpp>
#include "../process/ui_control.h"
#include "../process/config_control.h"
#include "frame_window.h"
#include "resource.h"
#include "api_message.h"
#include "encoding_changer.h"
#include <shlobj.h>
#include <stdlib.h>

using std::string;
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

	SetForegroundWindow(m_hWnd);                  // 解决托盘菜单失去焦点不会自动关闭的问题
	
	// Draw and track the shortcut menu.
	TrackPopupMenu(hmenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
		mouse_point.x, mouse_point.y, 0, m_hWnd, NULL); 

	// Destroy the menu.
	DestroyMenu(hmenu);
}


void frame_window::show_store_window() {
	CEditUI* changestorepath_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("changestorepathedit")));
	::ShellExecute(m_hWnd, _T("open"), changestorepath_edit->GetText().GetData(), NULL, NULL, SW_SHOW);
}


LRESULT frame_window::on_create(WPARAM wParam, LPARAM lParam, bool& handled) {
	CDialogBuilder builder;

	::SendMessage(m_hWnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)m_icon);

	m_pm.Init(m_hWnd);
	CControlUI* root = builder.Create(_T("frame.xml"), (UINT)0,  NULL, &m_pm);
	if (!root){
		MessageBox(NULL, _T("找不到所需的资源文件！请尝试重新下载安装软件。"), _T("错误"), MB_ICONERROR | MB_OK);
		::exit(0);
		return 0;
	}

	m_pm.AttachDialog(root);
	m_pm.AddNotifier(this);

	// 配置存有账号密码直接登录
	config_control& m_config_control = config_control::get_mutable_instance();
	string account = m_config_control.account_get();
	string password = m_config_control.password_get();
	
	if (account.length() > 0 && password.length() > 0) {
		ui_control& m_ui_control = ui_control::get_mutable_instance();
		m_ui_control.login(account, password);
	}
	
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
	case ID_TRAY_FOLDER:
		show_store_window();
		break;
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

	handled = false;
	return 0;
}


LRESULT frame_window::on_tray(WPARAM wParam, LPARAM lParam, bool& handled) {
	if (lParam == WM_RBUTTONUP) {
		popup_tray_menu();
	} else if (lParam == WM_LBUTTONDBLCLK) {
		show_store_window();
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
	make_notify(_T("downloadedit"), &frame_window::on_downloadedit_notify);
	make_notify(_T("nolimitupload"), &frame_window::on_nolimitupload_notify);
	make_notify(_T("limitupload"), &frame_window::on_limitupload_notify);
	make_notify(_T("uploadedit"), &frame_window::on_uploadedit_notify);

	CEditUI* changestorepath_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("changestorepathedit")));
	changestorepath_edit->SetEnabled(false);

	//TODO 根据用户配置设置
	CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
	download_edit->SetEnabled(false);
	CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
	upload_edit->SetEnabled(false);
}


static bool is_number(CStdString& str) {
	char* p;
	strtoul(str.GetData(), &p, 0);
	if (*p == 0)                      // 空字符串也认为是数字
		return true;
	else
		return false;
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
			ui_control& m_ui_control = ui_control::get_mutable_instance();
			m_ui_control.login(encoding_changer::ascii2utf8(account.GetData()),
							   encoding_changer::ascii2utf8(password.GetData()));
		}
	}

	return true;
}


bool frame_window::on_register_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("click")) {
		::MessageBox(NULL, _T("注册功能尚未开放，敬请期待！"), _T("提示"), MB_ICONINFORMATION | MB_OK);
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
		
		ui_control& m_ui_control = ui_control::get_mutable_instance();
		m_ui_control.change_store_path(encoding_changer::ascii2utf8(buffer));

		::CoTaskMemFree(pidl);
	}
	return true;
}


bool frame_window::on_nolimitdownload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
		download_edit->SetEnabled(false);
		
		ui_control& m_ui_control = ui_control::get_mutable_instance();
		m_ui_control.change_max_download_limit(0);
	}

	return true;
}


bool frame_window::on_limitdownload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* download_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("downloadedit")));
		download_edit->SetEnabled(true);
		
		CStdString text = download_edit->GetText();
		if (text.GetLength() > 0) {
			unsigned long limit = strtoul(text.GetData(), NULL, 0);
			limit *= 1024;
			
			ui_control& m_ui_control = ui_control::get_mutable_instance();
			m_ui_control.change_max_download_limit(limit);
		}
	}

	return true;
}


bool frame_window::on_downloadedit_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;
	
	if (pmsg->sType == _T("textchanged")) {
		CStdString text = pmsg->pSender->GetText();
		if (!is_number(text)) {
			pmsg->pSender->SetText(_T(""));
			return true;
		}
			
		if (text.GetLength() > 0) {
			unsigned long limit = strtoul(text.GetData(), NULL, 0);
			limit *= 1024;
			
			ui_control& m_ui_control = ui_control::get_mutable_instance();
			m_ui_control.change_max_download_limit(limit);
		}
	}
	
	return true;
}


bool frame_window::on_nolimitupload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
		upload_edit->SetEnabled(false);
		
		ui_control& m_ui_control = ui_control::get_mutable_instance();
		m_ui_control.change_max_upload_limit(0);
	}

	return true;
}


bool frame_window::on_limitupload_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;

	if (pmsg->sType == _T("selectchanged")) {
		CEditUI* upload_edit = dynamic_cast<CEditUI*>(m_pm.FindControl(_T("uploadedit")));
		upload_edit->SetEnabled(true);
		
		CStdString text = upload_edit->GetText();
		if (text.GetLength() > 0) {
			unsigned long limit = strtoul(text.GetData(), NULL, 0);
			limit *= 1024;
			
			ui_control& m_ui_control = ui_control::get_mutable_instance();
			m_ui_control.change_max_upload_limit(limit);
		}
	}

	return true;
}


bool frame_window::on_uploadedit_notify(void* msg) {
	TNotifyUI* pmsg = (TNotifyUI*)msg;
	
	if (pmsg->sType == _T("textchanged")) {
		CStdString text = pmsg->pSender->GetText();
		if (!is_number(text)) {
			pmsg->pSender->SetText(_T(""));
			return true;
		}
		
		if (text.GetLength() > 0) {
			unsigned long limit = strtoul(text.GetData(), NULL, 0);
			limit *= 1024;
			
			ui_control& m_ui_control = ui_control::get_mutable_instance();
			m_ui_control.change_max_upload_limit(limit);
		}
	}
	
	return true;
}
