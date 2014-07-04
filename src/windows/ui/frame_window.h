#pragma once
#include <boost/assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_container.hpp>
#include <boost/serialization/singleton.hpp>
#include <DuiLib/StdAfx.h>
#include "api_message.h"

namespace psychokinesis {

class frame_window : public DuiLib::CWindowWnd, public DuiLib::INotifyUI, public boost::serialization::singleton<frame_window> {
public:
	frame_window();

	LPCTSTR GetWindowClassName() const {
		return _T("UIMainFrame"); 
	}

	UINT GetClassStyle() const {
		return CS_DBLCLKS;
	}

	void Notify(DuiLib::TNotifyUI& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool create();
	bool handle_background_message();
	void destory();
	void show_tray_tip(const DuiLib::CStdString& tip_title, const DuiLib::CStdString& tip_msg);
	void post_message(api_message* msg);
	
	DuiLib::CPaintManagerUI m_pm;

private:
	void make_notify(const DuiLib::CStdString& control_name, bool (frame_window::* event_func)(void*)) {
		DuiLib::CSliderUI* psilder = static_cast<DuiLib::CSliderUI*>(m_pm.FindControl(control_name));

		BOOST_ASSERT(psilder && "control_name is NULL!");

		DuiLib::CDelegate<frame_window, frame_window> d = DuiLib::MakeDelegate(this, event_func);
		psilder->OnNotify += d;
	}

	// 托盘图标接口
	bool add_tray();
	void delete_tray();
	void popup_tray_menu();

	void show_store_window();
	
	// 原始窗口事件
	LRESULT on_create(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_close(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_destory(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_syscommand(WPARAM wParam, LPARAM lParam, bool& handled);            // 窗口菜单命令
	LRESULT on_command(WPARAM wParam, LPARAM lParam, bool& handled);               // 自建菜单命令
	LRESULT on_tray(WPARAM wParam, LPARAM lParam, bool& handled);

	// directui控件事件
	void on_windowinit_notify(DuiLib::TNotifyUI& msg);
	bool on_tab_notify(void* msg);
	bool on_login_notify(void* msg);
	bool on_register_notify(void* msg);
	bool on_changestorepath_notify(void* msg);
	bool on_nolimitdownload_notify(void* msg);
	bool on_limitdownload_notify(void* msg);
	bool on_downloadedit_notify(void* msg);
	bool on_nolimitupload_notify(void* msg);
	bool on_limitupload_notify(void* msg);
	bool on_uploadedit_notify(void* msg);

	boost::unordered_map<std::string, int> tabs;
	boost::ptr_deque<api_message> api_msgs;
	OSVERSIONINFO windows_version;
	NOTIFYICONDATA m_tray;
	HICON m_icon;
};

} // namespace psychokinesis