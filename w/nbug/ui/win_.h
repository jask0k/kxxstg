#pragma once
#include <nbug/tl/map.h>

#ifdef NB_LINUX
#   include <X11/Xlib.h>
#   include <X11/Xatom.h>
#endif

#include <nbug/core/def.h>
#include <nbug/core/file.h>
#include <nbug/gl/tex.h>
#include <nbug/ui/win.h>

namespace e
{

#ifdef NB_WINDOWS
	typedef HWND NativeWinID;
#endif

#ifdef NB_LINUX
	typedef Window NativeWinID;
#endif

	class Pane;
	class PaneStack;
	class Graphics;
	struct Win_o
	{
		Win * wrapper;
		//Graphics * graphics;

#ifdef NB_WINDOWS
		HWND hWnd;
		static LRESULT CALLBACK _WindowProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);
#endif

#ifdef NB_LINUX
		NativeWinID hWnd;
		static void _WindowProc(XEvent & _event);
#endif
		//Win * graphics;
		int backupScreenWidth;
		int backupScreenHeight;

		uint logicFps;
		WinFormat format;
		static LoopState PumpMessage();
		bool CreateGraphics();
		void GetScreenSize(int & _w, int & _h);
		void RenderBlackWin();

		//Pane * oldHover; // 
		//float hoverFrac;

		//void PushGrabMousePane(Pane * _w);
		//void PopGrabMousePane(Pane * _w, bool _topOnly = false);
		//void PushFocusPane(Pane * _w);
		//void PopFocusPane(Pane * _w, bool _topOnly = false);
		Win_o(Win * _wrapper);
		~Win_o();
	};
}
