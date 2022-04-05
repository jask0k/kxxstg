#include <nbug/core/debug.h>

#ifdef NB_WINDOWS

#include <windows.h>
#include <commctrl.h>
#include <nbug/core/ini.h>
#include <nbug/core/env.h>
#include <nbug/core/translate.h>
#include <z_kxx/main/options.h>
#include <z_kxx/main/resource.h>

namespace e
{
	static KxxOptions * options = 0;
	//static HICON g_hIconSmall = 0;
	//static HICON g_hIconBig = 0;
	static int g_engine = ENGINE_OPENGL;

	static WNDPROC defaultButtonProc = 0;
	static BOOL CALLBACK MyButtonFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch(msg)
		{
		case WM_KEYDOWN:
		case WM_KEYUP:
			if(wParam == 'Z' || wParam == 'z')
			{
				wParam = VK_SPACE;
			}
			break;
		}
		return defaultButtonProc(hWnd, msg, wParam, lParam);
	}

	static BOOL CALLBACK DialogFunc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg) 
		{
		case WM_INITDIALOG:
			{
				::SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(100)));
				::SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, 
					(LPARAM)(HICON)::LoadImage(GetModuleHandle(NULL),
					MAKEINTRESOURCE(100),
					IMAGE_ICON,
					16,
					16,
					LR_DEFAULTCOLOR));
				string title = _TT("KXX01");
				string msg = _TT("Select graphics engine") + L":";
				::SetWindowText(hWnd, title.c_str());
				::SetWindowText(::GetDlgItem(hWnd, ID_STATIC_PROMPT), msg.c_str());

				RECT rect;
				::GetWindowRect(hWnd, &rect);
				int x = options->windowLeft;
				int y = options->windowTop;
				int w = rect.right - rect.left;
				int h = rect.bottom - rect.top;
				int cx = ::GetSystemMetrics(SM_CXSCREEN);
				int cy = ::GetSystemMetrics(SM_CYSCREEN);
				int xmax = cx - w;
				int ymax = cy - h;
				if(x < 0) 
				{
					x = 0;
				}
				if(y < 0) 
				{
					y = 0;
				}
				if(x > xmax) 
				{
					x = xmax;
				}
				if(y > ymax) 
				{
					y = ymax;
				}
				::MoveWindow(hWnd, x, y, w, h, TRUE);
				//::ShowWindow(hWnd, SW_SHOW);
				HWND h0 = ::GetDlgItem(hWnd, ENGINE_OPENGL);
				defaultButtonProc = (WNDPROC)::SetWindowLongPtr(h0, GWL_WNDPROC, (LONG_PTR)MyButtonFunc);
				HWND h1 = ::GetDlgItem(hWnd, ENGINE_DIRECT3D);
				::SetWindowLongPtr(h1, GWL_WNDPROC, (LONG_PTR)MyButtonFunc);
				if(g_engine == ENGINE_DIRECT3D)
				{
					SetFocus(h1);
					::SetWindowLong(h0, GWL_STYLE, ::GetWindowLong(h0, GWL_STYLE)& ~BS_DEFPUSHBUTTON);
					::SetWindowLong(h1, GWL_STYLE, ::GetWindowLong(h1, GWL_STYLE)| BS_DEFPUSHBUTTON);
					return 0;
				}
				return 1;
			}
			break;
		case WM_COMMAND:
			g_engine = (int)wParam;
			EndDialog(hWnd, IDOK);
			break;
		case WM_CLOSE:
			EndDialog(hWnd, IDCANCEL);
			return 1;
		}
		return 0;
	}

	void ShowConfigDialog(bool _crashed)
	{
		InitCommonControls();

		options = enew KxxOptions();
		options->Load();

		if(_crashed)
		{
			g_engine = options->graphicsBackend ? ENGINE_OPENGL : ENGINE_DIRECT3D;
		}

		::DialogBox(::GetModuleHandle(NULL), MAKEINTRESOURCE(200), NULL, (DLGPROC) DialogFunc);
	
		int newEngine = g_engine == ENGINE_OPENGL ? 0 : 1;
		if(options->graphicsBackend != newEngine)
		{
			options->graphicsBackend = newEngine;
			options->Save();
		}

		delete options;
		options = 0;
	}
}

#else

namespace e
{	
	void ShowConfigDialog(bool _b)
	{
		message(L"[kx] (WW) unimplemented.");
	}
	
}

#endif // NB_WINDOWS
