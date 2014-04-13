#include "api_message.h"
#include "frame_window.h"

using DuiLib::CPaintManagerUI;
using DuiLib::CButtonUI;
using DuiLib::CEditUI;

using namespace psychokinesis;

void api_communication_logged::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;

	CButtonUI* login_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("loginbtn")));
	login_button->SetText(_T("ÒÑµÇÂ¼"));
}


void api_communication_login_failed::execute() {
	CPaintManagerUI& window_manager = frame_window::get_mutable_instance().m_pm;
	CButtonUI* login_button = dynamic_cast<CButtonUI*>(window_manager.FindControl(_T("loginbtn")));
	
	if (!login_button->IsEnabled()) {
		MessageBox(NULL, _T("µÇÂ¼Ê§°Ü!"), _T("´íÎó"), MB_ICONERROR | MB_OK);

		CEditUI* account_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("accountedit")));
		CEditUI* password_edit = dynamic_cast<CEditUI*>(window_manager.FindControl(_T("pwdedit")));
		account_edit->SetEnabled(true);
		password_edit->SetText(_T(""));
		password_edit->SetEnabled(true);

		login_button->SetEnabled(true);
	}
}
