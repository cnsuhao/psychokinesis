#include <sstream>
#include "../process/ui_control.h"
#include "api_message.h"
#include "frame_window.h"
#include "encoding_changer.h"


using std::stringstream;
using DuiLib::CPaintManagerUI;
using DuiLib::CButtonUI;
using DuiLib::CEditUI;
using DuiLib::CStdString;
using DuiLib::COptionUI;

using namespace psychokinesis;

void api_communication_logging::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CButtonUI* login_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("loginbtn")));
	CEditUI* account_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("accountedit")));
	CEditUI* password_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("pwdedit")));
	
	login_button->SetEnabled(false);
	login_button->SetText(_T("登录中"));
	
	account_edit->SetEnabled(false);
	account_edit->SetText(encoding_changer::utf82ascii(account).c_str());
	password_edit->SetEnabled(false);
	password_edit->SetText(password.c_str());
}


void api_communication_logged::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CButtonUI* login_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("loginbtn")));
	CEditUI* account_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("accountedit")));
	CEditUI* password_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("pwdedit")));
	CButtonUI* register_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("registerbtn")));
	
	login_button->SetEnabled(false);
	login_button->SetText(_T("已登录"));
	register_button->SetEnabled(false);
	
	account_edit->SetEnabled(false);
	account_edit->SetText(encoding_changer::utf82ascii(account).c_str());
	password_edit->SetEnabled(false);
	password_edit->SetText(password.c_str());
}


void api_communication_login_failed::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CButtonUI* login_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("loginbtn")));
	
	switch (error_code) {
	case ui_control::authentication_failed:
		{
			MessageBox(NULL, _T("账号或密码错误!"), _T("错误"), MB_ICONERROR | MB_OK);
			break;
		}
	case ui_control::connect_failed:
		{
			MessageBox(NULL, _T("登录失败，请确认电脑能够连接Internet。"), _T("错误"), MB_ICONERROR | MB_OK);
			break;
		}
	default:
		{
			return;                                   // 连接中断时会自动进行重连
		}
	}

	CEditUI* account_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("accountedit")));
	CEditUI* password_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("pwdedit")));
	account_edit->SetEnabled(true);
	password_edit->SetText(_T(""));
	password_edit->SetEnabled(true);

	login_button->SetText(_T("登录"));
	login_button->SetEnabled(true);
}


void api_download_store_path_changed::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CEditUI* changestorepath_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("changestorepathedit")));
	
	changestorepath_edit->SetText(encoding_changer::utf82ascii(new_path).c_str());
}


void api_download_max_download_limit_changed::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CEditUI* download_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("downloadedit")));
	COptionUI* limitdownload_option = dynamic_cast<COptionUI*>(window_manager.FindControl(_T("limitdownload")));
	
	unsigned int kb_limit = limit / 1024;
	CStdString str_kb_limit;
	str_kb_limit.Format("%u", kb_limit);
	if (download_edit->GetText() != str_kb_limit) {
		download_edit->SetText(str_kb_limit);
		
		if (!limitdownload_option->IsSelected() && kb_limit > 0)
			limitdownload_option->Selected(true);
	}
}


void api_download_max_upload_limit_changed::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CEditUI* upload_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("uploadedit")));
	COptionUI* limitupload_option = dynamic_cast<COptionUI*>(window_manager.FindControl(_T("limitupload")));
	
	unsigned int kb_limit = limit / 1024;
	CStdString str_kb_limit;
	str_kb_limit.Format("%u", kb_limit);
	if (upload_edit->GetText() != str_kb_limit) {
		upload_edit->SetText(str_kb_limit);
		
		if (!limitupload_option->IsSelected() && kb_limit > 0)
			limitupload_option->Selected(true);
	}
}
