#pragma once
#include <boost/unordered_map.hpp>
#include <DuiLib/StdAfx.h>

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

public:
	DuiLib::CPaintManagerUI m_pm;

private:
	LRESULT on_create(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_close(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_destory(WPARAM wParam, LPARAM lParam, bool& handled);
	LRESULT on_syscommand(WPARAM wParam, LPARAM lParam, bool& handled);

	boost::unordered_map<std::string, int> tabs;
};

} // namespace psychokinesis