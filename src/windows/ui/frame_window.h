#pragma once
#include <boost/assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_container.hpp>
#include <DuiLib/StdAfx.h>
#include "api_message.h"

namespace psychokinesis {

class frame_window : public DuiLib::CWindowWnd, public DuiLib::INotifyUI
{
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
	void handle_background_message();

	DuiLib::CPaintManagerUI m_pm;

private:
	void make_notify(const DuiLib::CStdString& control_name, bool (frame_window::* event_func)(void*)) {
		DuiLib::CSliderUI* psilder = static_cast<DuiLib::CSliderUI*>(m_pm.FindControl(control_name));

		BOOST_ASSERT(psilder && "control_name is NULL!");

		DuiLib::CDelegate<frame_window, frame_window> d = DuiLib::MakeDelegate(this, event_func);
		psilder->OnNotify += d;
	}

	// 原始窗口事件
	LRESULT on_create(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_close(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_destory(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_syscommand(WPARAM wParam, LPARAM lParam, bool& handled);

	// directui控件事件
	bool on_tab_notify(void* msg);
	bool on_login_notify(void* msg);

	boost::unordered_map<std::string, int> tabs;
	boost::ptr_deque<api_message> api_msgs;
};

} // namespace psychokinesis