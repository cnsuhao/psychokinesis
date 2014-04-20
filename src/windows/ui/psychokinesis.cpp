#include <DuiLib/StdAfx.h>
#include "frame_window.h"
#include "../process/control.h"

using DuiLib::CPaintManagerUI;

using namespace psychokinesis;

// ����һ��Ψһ�Ĵ���������������ʵ�ֵ�ʵ������
TCHAR g_window_prop_name[] = _T("57d0f19d-74c5-4ded-b6ad-0b085fe70b0f");
HANDLE g_window_value = (HANDLE)1022;

void window_message_loop();
BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPSTR /*lpCmdLine*/, int nCmdShow){
	// ��ⵥʵ������
	HWND old_hwnd = NULL;
	::EnumWindows(enum_wnd_proc,(LPARAM)&old_hwnd);    // ö���������еĴ���
	if (old_hwnd != NULL) {
		::MessageBox(NULL, _T("������������!"), _T("����"), MB_ICONERROR | MB_OK);
		return 0;
	}
	
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("res"));
	
	// ��ʼ��COM
	HRESULT Hr = ::CoInitialize(NULL);                // ���ڽ����߳�
	if (FAILED(Hr)) {
		MessageBox(NULL, _T("��ʼ��COMʧ��!"), _T("����"), MB_ICONERROR | MB_OK);
		return 0;
	}
	
	do {
		// ��ʼ����̨
		control& m_control = control::get_mutable_instance();
		if (!m_control.open()) {
			MessageBox(NULL, _T("�����ʼ��ʧ��!�볢���������ذ�װ�����"), _T("����"), MB_ICONERROR | MB_OK);
			break;
		}
	
		// ��ʼ������
		frame_window& m_window = frame_window::get_mutable_instance();
		if (!m_window.create()) {
			MessageBox(NULL, _T("�����ʼ��ʧ��!�볢���������ذ�װ�����"), _T("����"), MB_ICONERROR | MB_OK);
			m_control.close();
			break;
		}
		
		// �����ڴ��������ã�Ϊ���ڸ���һ�����ԣ�����ʵ�ֵ�ʵ������
		::SetProp(m_window.GetHWND(), g_window_prop_name, g_window_value);
		
		window_message_loop();
	
		// �������˳�ʱ�Ƴ��ø������ԣ�����ʵ�ֵ�ʵ������
		::RemoveProp(m_window.GetHWND(), g_window_prop_name);
		
		m_window.destory();
		m_control.close();
	} while (0);
	
	::CoUninitialize();
	
	return 0;
}


void window_message_loop() {
	frame_window& window = frame_window::get_mutable_instance();
	MSG msg = {0};

	for (;;) {
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (!CPaintManagerUI::TranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
				return;
		}

		if (!window.handle_background_message())
			::WaitMessage();
	}
}

BOOL CALLBACK enum_wnd_proc(HWND hwnd, LPARAM lparam) {
	HANDLE h = ::GetProp(hwnd, g_window_prop_name);
	if(h == g_window_value)
	{
		*(HWND*)lparam = hwnd;
		return FALSE;
	}
	return TRUE;
}
