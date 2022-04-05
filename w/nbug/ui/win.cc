#include <string.h>
#include <nbug/gl/private.h>
#include <nbug/tl/map.h>
//
//#ifdef E_CFG_DIRECT3D
//#   ifdef _MSC_VER
//#	    include <DXErr.h>
//#   else
//#       include <dxerr9.h>
//#   endif
//#endif

#include <nbug/core/def.h>
#include <nbug/tl/array.h>
#include <nbug/core/str.h>
#include <nbug/core/debug.h>
#include <nbug/core/obj.h>
#include <nbug/core/env.h>
#include <nbug/input/keyboard.h>
#include <nbug/core/thread.h>
#include <nbug/core/time.h>
#include <nbug/ui/win.h>
#include <nbug/ui/win_.h>
#include <nbug/gl/fbo.h>
#ifdef E_CFG_OPENGL
#	include <nbug/gl/gl_exts.h>
#endif
#include <nbug/gl/font/font.h>
#include <nbug/gl/font/font_.h>
#include <nbug/gl/color.h>
#include <nbug/math/math.h>
#include <nbug/gl/graphics.h>
#include <nbug/gl/graphics_.h>
#include <nbug/ui/pane.h>
#include <nbug/ui/pane_.h>
#include <nbug/ui/pane_stack.h>
#include <nbug/core/dll_load.h>

namespace e
{
	extern PaneStack * g_pane_stack;
	extern Pane * g_keyboard_focus_pane;
	extern Pane * g_grab_mouse_pane;
	extern Array<Pane *> * g_keyboard_focus_pane_stack;
	extern Array<Pane *> * g_grab_mouse_pane_stack;
	extern Pane * g_mouse_hover_pane;
	//extern List<Pane*> g_noparent_pane_list;

	//static Win * singleton = 0;
	Win * Win::singleton = 0;
	bool g_to_delete_win_singleton = false;
	WinFormat::WinFormat()
	{
		allowResize = true;
		maximized = false;
		x = 100;
		y = 100;
		w = 640;
		h = 480;
		fullScreen = false;
	}

	GraphicsFormat::GraphicsFormat()
	{
		vsync= true;
		doubleBuffer = true;
		zBufferBits = 16; // 16
		alphaBufferBits = 8; // 8
		stencilBufferBits = 0; // 8
	}


#ifdef NB_LINUX
	Display * _display = 0;
	int _screenNum = 0;
	Colormap _screenColormap;
	typedef ::Window WindowID;
	typedef e::Map<WindowID, Win*> WindowMap;
	static WindowMap g_window_map;
	Atom _atom_WM_PROTOCOLS = None;
	Atom _atom_WM_DELETE_WINDOW = None;
	Atom _atom_E_WM_PENDING_CALL = None;
	Atom _atom_E_WM_CLOSE = None;
	Atom _atom_UTF8_STRING = None;
	Atom _atom_CLIPBOARD = None;
	Atom _atom_E_SELECTION = None;
	Atom _atom_TARGETS = None;
	Atom _atom_NET_WM_PING = None;
	Atom _atom_NET_WM_ICON = None;
	Atom _atom_NET_WM_STATE = None;
	Atom _atom_NET_WM_STATE_MODAL = None;
	Atom _atom_NET_WM_STATE_NORMAL = None;
	Atom _atom_NET_WM_STATE_ADD = None;
	Atom _atom_NET_WM_STATE_REMOVE = None;
	Atom _atom_NET_WM_STATE_MAXIMIZED_VERT = None;
	Atom _atom_NET_WM_STATE_MAXIMIZED_HORZ = None;
	Atom _atom_NET_WM_STATE_SHADED = None;
	Atom _atom_NET_WM_STATE_HIDDEN = None;
	Atom _atom_NET_WM_STATE_FULLSCREEN = None;
	static void _InitXLib()
	{
		if(_display == 0)
		{
			if((_display = XOpenDisplay(NULL)) == NULL) // 248 bytes in 2 blocks are definitely lost in loss record 230 of 354 [PID: 26939]	win.cc	/w/nbug/gl/ui	line 104	Valgrind Problem

			{
				E_ASSERT1(0, "[nb] Failed to open display.");
				write_log( "[nb] (EE) Failed to open display.");
				exit(1);
			}
			_screenNum = DefaultScreen(_display);
			_screenColormap = DefaultColormap(_display, DefaultScreen(_display));
		}

		_atom_WM_DELETE_WINDOW  = XInternAtom(_display, "WM_DELETE_WINDOW",False);
		_atom_E_WM_PENDING_CALL = XInternAtom(_display, "E_WM_PENDING_CALL",False);
		_atom_E_WM_CLOSE        = XInternAtom(_display, "E_WM_CLOSE",False);
		_atom_WM_PROTOCOLS      = XInternAtom(_display, "WM_PROTOCOLS", False);
		_atom_UTF8_STRING       = XInternAtom(_display, "UTF8_STRING", False);
		_atom_CLIPBOARD         = XInternAtom(_display, "CLIPBOARD", False);
		_atom_E_SELECTION       = XInternAtom(_display, "E_SELECTION", False);
		_atom_TARGETS           = XInternAtom(_display, "TARGETS", False);
		_atom_NET_WM_PING       = XInternAtom(_display, "_NET_WM_PING", False);
		_atom_NET_WM_ICON       = XInternAtom(_display, "_NET_WM_ICON", False);
		_atom_NET_WM_STATE      = XInternAtom(_display, "_NET_WM_STATE", False);
		_atom_NET_WM_STATE_MODAL= XInternAtom(_display, "_NET_WM_STATE_MODAL", False);
		_atom_NET_WM_STATE_NORMAL= XInternAtom(_display, "_NET_WM_STATE_NORMAL", False);
		_atom_NET_WM_STATE_ADD   = XInternAtom(_display, "_NET_WM_STATE_ADD", False);
		_atom_NET_WM_STATE_REMOVE= XInternAtom(_display, "_NET_WM_STATE_REMOVE", False);
		_atom_NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_atom_NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_atom_NET_WM_STATE_SHADED = XInternAtom(_display, "_NET_WM_STATE_SHADED", False);
		_atom_NET_WM_STATE_HIDDEN = XInternAtom(_display, "_WM_STATE_HIDDEN", False);
		_atom_NET_WM_STATE_FULLSCREEN = XInternAtom(_display, "_WM_STATE_FULLSCREEN", False);
	}

	static void _CloseXLib()
	{
		//::XCloseDisplay(_display);
		//_display = 0;
	}
#endif


	void Win_o::GetScreenSize(int & _w, int & _h)
	{
#ifdef NB_WINDOWS
		//RECT rc;
		//::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0);
		//_w = rc.right - rc.left;
		//_h = rc.bottom - rc.top;
		_w = ::GetSystemMetrics(SM_CXSCREEN);
		_h = ::GetSystemMetrics(SM_CYSCREEN);

#endif

#ifdef NB_LINUX
		_w = ::XDisplayWidth(_display, _screenNum);
		_h = ::XDisplayHeight(_display, _screenNum);
#endif
	}


	static bool GetDesktopArea(int & _x0, int & _y0, int & _x1, int & _y1)
	{
#ifdef NB_WINDOWS
		RECT rc;
		if(::SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&rc, 0))
		{
			_x0 = rc.left;
			_y0 = rc.top;
			_x1 = rc.right;
			_y1 = rc.bottom;
			return true;
		}
		else
		{
			return false;
		}

#endif

#ifdef NB_LINUX
		_x0 = 0;
		_y0 = 0;
		_x1 = ::XDisplayWidth(_display, _screenNum);
		_y1 = ::XDisplayHeight(_display, _screenNum);
		return true;
#endif
	}



	Win_o::Win_o(Win * _wrapper)
	{
		wrapper = _wrapper;
		hWnd = 0;
		//rootPane = 0;
		//g_pane_stack = 0;
		//g_keyboard_focus_pane = 0;
		//g_grab_mouse_pane = 0;
		//g_keyboard_focus_pane_stack = 0;
		//g_grab_mouse_pane_stack = 0;
		//g_mouse_hover_pane = 0;
//		oldHover = 0;
//		hoverFrac = 0;
	}

	Win_o::~Win_o()
	{
		//delete g_keyboard_focus_pane;
		//delete g_grab_mouse_pane;
		//delete _hover;
#ifdef NB_WINDOWS
		if(hWnd)
		{
			::SetWindowLongPtr(hWnd, GWL_USERDATA, 0);
			::DestroyWindow((HWND)hWnd);
		}

#endif // NB_WINDOWS

#ifdef NB_LINUX
		if(hWnd)
		{
			g_window_map.erase(hWnd);
			::XDestroyWindow(_display, hWnd);
		}
		_CloseXLib();
#endif // NB_LINUX
	}

	Win::Win()
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(singleton == 0);
		E_ASSERT(Graphics::Singleton() == 0);

		singleton = this;
		o = enew Win_o(this);

#ifdef NB_LINUX
		_InitXLib();
#endif
		o->GetScreenSize(o->backupScreenWidth, o->backupScreenHeight);
	}

	Win::~Win()
	{
		// NB_PROFILE_INCLUDE;

		E_ASSERT(singleton == this);
		singleton = 0;
		//E_ASSERT(!o->isDeviceOperational);

		Pane::SetRootPane(0);

		//for(auto it = g_noparent_pane_list.begin(); it != g_noparent_pane_list.end(); ++it)
		//{
		//	if(*it != Pane::GetRootPane())
		//	{
		//		delete *it;
		//	}
		//}


		E_SAFE_DELETE(Graphics::singleton);

//		delete g_keyboard_focus_pane_stack;
//		delete g_grab_mouse_pane_stack;
		delete o;
	}

#ifdef NB_WINDOWS

	LRESULT CALLBACK Win_o::_WindowProc(HWND _hWnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
	{
		Win * win = (Win*)(void*)::GetWindowLongPtr(_hWnd, GWL_USERDATA);
		if(win)
		{
			return win->WindowProc(_msg, _wParam, _lParam);
		}
		else if(_msg == WM_ERASEBKGND)
		{
			//
			// 
			RECT rect;
			::GetClientRect(_hWnd, &rect);
			::FillRect((HDC)_wParam, &rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
			return 0;
		}
		else
		{
			return ::DefWindowProc(_hWnd, _msg, _wParam, _lParam);
		}
	}

	BOOL WINAPI MyDelPropProc(HWND h, LPTSTR s, HANDLE, ULONG_PTR)
	{ 
		RemoveProp(h, s); 
		return TRUE; 
	} 

	int Win::WindowProc(uint _msg, uintx _wParam, intx _lParam)
	{
		// NB_PROFILE_INCLUDE;
		switch(_msg)
		{
		case WM_SIZE:
			if(_wParam != SIZE_MINIMIZED)
			{
				this->_OnSize((short)LOWORD(_lParam), (short)HIWORD(_lParam));
			}
			break;
		case WM_KEYDOWN:
			this->OnKeyDown((int)_wParam);
			break;
		case WM_KEYUP:
			this->OnKeyUp((int)_wParam);
			break;
		case WM_DESTROY:
			// HWND will destory by class Win befor we got WM_DESTROY.
			EnumPropsEx(o->hWnd, &MyDelPropProc, 0); 
			break;
		case (WM_USER+105):
			::ShowWindow(o->hWnd, SW_SHOW);
			//::BringWindowToTop(o->hWnd);
			::SetForegroundWindow(o->hWnd);
			break;
		case WM_MOUSEMOVE:
			this->OnMouseMove((short)LOWORD(_lParam), (short)HIWORD(_lParam));
			break;
		case WM_LBUTTONDOWN:
			::SetCapture(o->hWnd);
			this->OnLeftDown((short)LOWORD(_lParam), (short)HIWORD(_lParam));
			break;
		case WM_LBUTTONUP:
			this->OnLeftUp((short)LOWORD(_lParam), (short)HIWORD(_lParam));
			if(::GetCapture() == o->hWnd)
			{
				::ReleaseCapture();
			}
			break;
		case WM_MOUSEWHEEL:
			{
				POINT pt ={ (short)LOWORD(_lParam), (short)HIWORD(_lParam) };
				ScreenToClient(o->hWnd, &pt);
				this->OnMouseWheel((float)pt.x, (float)pt.y, (short)HIWORD(_wParam) / (float) WHEEL_DELTA);
			}
		case WM_PAINT:
			::ValidateRect(o->hWnd, NULL);
			if(Graphics::Singleton() == 0 || !Graphics::Singleton()->IsDeviceOperational())
			{
				// window bg is init as bright in windows7, we paint it with black
				this->o->RenderBlackWin();
			}
			break;
		case WM_CLOSE:
			this->SafeDeleteThis();
			break;
		default:
			return ::DefWindowProc(o->hWnd, _msg, _wParam, _lParam);
		}
		return 0;
	}

	static void _UnregisterClass()
	{
		::UnregisterClass(L"NIGHTBUG_WIN", NULL);
	}

	bool Win::Create(const WinFormat & _format, const Char * _unique_prop_name)
	{
		// NB_PROFILE_INCLUDE;
		o->format = _format;
		static bool _initialized = false;
		if(!_initialized)
		{
			WNDCLASSEX wc;
			memset(&wc, 0, sizeof(wc));
			wc.cbSize = sizeof (WNDCLASSEX);
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = L"NIGHTBUG_WIN";
			wc.lpfnWndProc = Win_o::_WindowProc;
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.hIcon =  ::LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(100));
			if(wc.hIcon == NULL)
			{
				wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
			}
			wc.hIconSm = (HICON)::LoadImage(GetModuleHandle(NULL),
				MAKEINTRESOURCE(100),
				IMAGE_ICON,
				16,
				16,
				LR_DEFAULTCOLOR);
			if(wc.hIconSm == NULL)
			{
				wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
			}
			wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			wc.lpszMenuName = NULL;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = sizeof(Win*);
			wc.hbrBackground = NULL;

			if(RegisterClassEx (&wc))
			{
				_initialized = true;
				atexit(&_UnregisterClass);
			}
			else
			{
				return false;
			}
		}

		if(o->hWnd)
		{
			E_ASSERT(0);
			return false;
		}


		DWORD ws = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
		//DWORD ws = WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE;
		if(!_format.allowResize)
		{
			ws&= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
		}

		//{
		//int x0, y0, x1, y1;
		//if(GetDesktopArea(x0, y0, x1, y1))
		//{
		//	RECT rect;
		//	::GetClientRect(_win, &rect);
		//	DWORD ws = ::GetWindowLong(_win, GWL_STYLE);
		//	::AdjustWindowRectEx(&rect, ws, FALSE, 0);
		//	if(_x + rect.right - rect.left > x1)
		//	{
		//		_x = x1 - (rect.right - rect.left);
		//	}
		//	if(_y + rect.bottom - rect.top > y1)
		//	{
		//		_y = y1 - (rect.bottom - rect.top);
		//	}
		//	if(_x < x0)
		//	{
		//		_x = x0;
		//	}
		//	if(_y < y0)
		//	{
		//		_y = y0;
		//	}
		//}


		int & x = o->format.x;
		int & y = o->format.y;

		// calc proper size
		int w, h;
		{
			RECT rect;
			rect.left = 20;
			rect.top = 20;
			rect.right = rect.left+_format.w;
			rect.bottom = rect.top+_format.h;
			::AdjustWindowRectEx(&rect, ws, FALSE, 0);
			w = rect.right - rect.left;
			h = rect.bottom - rect.top;
		}

		// retrict whithin desktop
		int x0, y0, x1, y1;
		if(GetDesktopArea(x0, y0, x1, y1))
		{
			if(x + w > x1)
			{
				x = x1 - w;
			}
			if(y + h > y1)
			{
				y = y1 - h;
			}
			if(x < x0)
			{
				x = x0;
			}
			if(y < y0)
			{
				y = y0;
			}
		}

		o->hWnd = ::CreateWindowEx(
			WS_EX_APPWINDOW,
			L"NIGHTBUG_WIN",
			Env::GetShortName().c_str(),
			ws,
			x, y, w, h,
			NULL,
			NULL,
			::GetModuleHandle(NULL),
			0);
		if(o->hWnd == NULL)
		{
			return false;
		}

		if(_unique_prop_name)
		{
			BOOL b = ::SetProp(o->hWnd, _unique_prop_name, (HANDLE)1);
			E_ASSERT(b);
		}

		o->format.fullScreen = false;
		Resize(o->format.w, o->format.h, _format.fullScreen);

		//if(_CreateGraphics())
		//{
		//	_OnSize(o->format.w, o->format.h);
		//	return true;
		//}
		//else
		//{
		//	::DestroyWindow(o->hWnd);
		//	o->hWnd = 0;
		//	return false;
		//}
		return true;
	}

	Graphics * Win::CreateGraphics(const GraphicsFormat & _gf)
	{
		HDC hDC = ::GetDC((HWND)o->hWnd);
		if(hDC == NULL)
		{
			return 0;
		}

#ifdef E_CFG_OPENGL
		HGLRC hGL = 0;
#endif

#ifdef E_CFG_DIRECT3D
		D3DPRESENT_PARAMETERS d3dpp;
		LPDIRECT3D9        direct3d;
		LPDIRECT3DDEVICE9  device;
#endif

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			static PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
				1,                              // version number
				PFD_DRAW_TO_WINDOW           // support window
					| PFD_SUPPORT_OPENGL          // support OpenGL
					| PFD_DOUBLEBUFFER,             // double buffered
				PFD_TYPE_RGBA,                   // RGBA type
				24,                             // 24-bit color depth
				0, 0, 0, 0, 0, 0,               // color bits ignored
				_gf.alphaBufferBits,                              // alpha buffer
				0,                              // shift bit ignored
				0,                              // no accumulation buffer
				0, 0, 0, 0,                     // accum bits ignored
				_gf.zBufferBits,                             // z-buffer
				_gf.stencilBufferBits,                              // stencil buffer
				0,                              // no auxiliary buffer
				PFD_MAIN_PLANE,                 // main layer
				0,                              // reserved
				0, 0, 0                         // layer masks ignored
			};

			int pixelformat;
			if((pixelformat = ChoosePixelFormat(hDC, &pfd)) == 0
				|| SetPixelFormat(hDC, pixelformat, &pfd) == FALSE
				|| (hGL = wglCreateContext(hDC)) == NULL)
			{
				::ReleaseDC(o->hWnd, hDC);
				hDC = 0;
				::DestroyWindow(o->hWnd);
				o->hWnd = 0;
				return 0;
			}
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			void * hDll = hex_dll_open("d3d9.dll");
			if(hDll == NULL)
			{
				error(L"[nb] (EE) " + _TT("Fail to load d3d9.dll"));
				return 0;
			}
			typedef IDirect3D9* (WINAPI *FUNC_Direct3DCreate9)(UINT);
			FUNC_Direct3DCreate9 funcDirect3DCreate9 = (FUNC_Direct3DCreate9)::hex_dll_get_symbol(hDll, "Direct3DCreate9");
			if(funcDirect3DCreate9 == 0)
			{
				error(L"[nb] (EE) " + _TT("Fail to load func Direct3DCreate9."));
				return 0;
			}

			direct3d = funcDirect3DCreate9(D3D_SDK_VERSION);
			if(NULL == direct3d)
			{
				return 0;
			}

			memset(&d3dpp, 0, sizeof(d3dpp));
			d3dpp.Windowed = /*format.fullScreen ? FALSE :*/ TRUE;
			d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP;
			d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
			d3dpp.BackBufferCount = 1;
			//d3dpp.BackBufferFormat = D3DFMT_A8B8G8R8;
			d3dpp.BackBufferWidth = o->format.w;
			d3dpp.BackBufferHeight = o->format.h;
			d3dpp.MultiSampleQuality = D3DMULTISAMPLE_NONE;
			d3dpp.hDeviceWindow = o->hWnd;
			d3dpp.PresentationInterval = _gf.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
			//d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
			//d3dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;
			//d3dpp.MultiSampleQuality = 0;
			if(_gf.zBufferBits) // TODO: ....
			{
				d3dpp.EnableAutoDepthStencil = TRUE;
				d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
			}

			HRESULT hr = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, o->hWnd,
												D3DCREATE_MIXED_VERTEXPROCESSING
											  //D3DCREATE_SOFTWARE_VERTEXPROCESSING
											  //D3DCREATE_HARDWARE_VERTEXPROCESSING
												| D3DCREATE_FPU_PRESERVE
											  ,&d3dpp, &device);
			if(FAILED(hr) && !d3dpp.Windowed)
			{
				o->format.fullScreen = false;
				d3dpp.Windowed = TRUE;
				hr = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, o->hWnd,
											D3DCREATE_HARDWARE_VERTEXPROCESSING
												| D3DCREATE_FPU_PRESERVE
											,&d3dpp, &device);
			}

			if(FAILED(hr))
			{
				// try the solfware way
				d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
				hr = direct3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, o->hWnd,
											D3DCREATE_SOFTWARE_VERTEXPROCESSING
												| D3DCREATE_FPU_PRESERVE
											,&d3dpp, &device);
			}

			if(FAILED(hr))
			{
				E_ASSERT1(0, L"DXErr: " + string((uintx)hr));
				E_SAFE_RELEASE(direct3d);
				return 0;
			}

#endif
		}

		Graphics * & g = Graphics::singleton;
		g = enew Graphics();
		g->imp->win = this;
		g->imp->hDC = hDC;
		g->imp->winW = o->format.w;
		g->imp->winH = o->format.h;
		g->imp->viewW = o->format.w;
		g->imp->viewH = o->format.h;
		g->imp->gf = _gf;

		if(g_is_opengl)
		{
#ifdef E_CFG_OPENGL
			g->imp->hGL = hGL;
#endif
		}
		else
		{
#ifdef E_CFG_DIRECT3D
			g->imp->d3d = direct3d;
			g->imp->device = device;
			g->imp->d3dpp = d3dpp;
#endif
		}
		g->_OnCreateDevice();

		::SetWindowLongPtr(o->hWnd, GWL_USERDATA, (UINT_PTR)this);

		RECT rc;
		::GetClientRect(o->hWnd, &rc);
		OnSize(rc.right, rc.bottom);
		return g;
	}


	bool Win::Resize(int _w, int _h, bool _fullScreen)
	{
		RECT rect;
		::GetClientRect(o->hWnd, &rect);
		int old_w = rect.right;
		int old_h = rect.bottom;

		if(old_w == _w && old_h == _h && o->format.fullScreen == _fullScreen)
		{
			return true;
		}


		if(o->format.fullScreen != _fullScreen)
		{
			if(_fullScreen)
			{
				DWORD wsEx = WS_EX_TOPMOST | WS_EX_APPWINDOW;
				DWORD ws = WS_POPUP | WS_VISIBLE;
				::SetWindowLong(o->hWnd, GWL_EXSTYLE, wsEx);
				::SetWindowLong(o->hWnd, GWL_STYLE, ws);
				::SetWindowPos(o->hWnd, 0, 0, 0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				DEVMODE dm;
				memset(&dm, 0, sizeof(dm));
				dm.dmSize = sizeof(dm);
				dm.dmPelsWidth = _w;
				dm.dmPelsHeight = _h;
				dm.dmBitsPerPel = 32;
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
				{
					// TODO: error handling
				}
			}
			else
			{
				DWORD wsEx = WS_EX_APPWINDOW;
				DWORD ws = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
				::SetWindowLong(o->hWnd, GWL_EXSTYLE, wsEx);
				::SetWindowLong(o->hWnd, GWL_STYLE, ws);
				::SetWindowPos(o->hWnd, 0, 0, 0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

				// Windows API full screen
				DEVMODE dm;
				memset(&dm, 0, sizeof(dm));
				dm.dmSize = sizeof(dm);
				dm.dmPelsWidth = o->backupScreenWidth;
				dm.dmPelsHeight = o->backupScreenHeight;
				dm.dmBitsPerPel = 32;
				dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

				if(ChangeDisplaySettings(&dm, CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
				{
					// TODO: error handling
				}
			}
		}

		o->format.w = _w;
		o->format.h = _h;
		o->format.fullScreen = _fullScreen;

		if(_fullScreen)
		{
			::MoveWindow(o->hWnd, 0, 0, _w, _h, FALSE);
		}
		else
		{
			DWORD ws = ::GetWindowLong(o->hWnd, GWL_STYLE);
			int x, y, w, h;
			x = o->format.x;
			y = o->format.y;
			rect.left = 100;
			rect.top = 100;
			rect.right = rect.left + o->format.w;
			rect.bottom = rect.top + o->format.h;
			::AdjustWindowRectEx(&rect, ws, FALSE, 0);
			w = rect.right - rect.left;
			h = rect.bottom - rect.top;

			int x0, y0, x1, y1;
			if(GetDesktopArea(x0, y0, x1, y1))
			{
				if(x + w > x1)
				{
					x = x1 - w;
				}
				if(y + h > y1)
				{
					y = y1 - h;
				}
				if(x < x0)
				{
					x = x0;
				}
				if(y < y0)
				{
					y = y0;
				}
			}

			::MoveWindow(o->hWnd, x, y, w, h, FALSE);
		}

#ifdef E_CFG_DIRECT3D
		o->RenderBlackWin();
		//return _CreateGraphics();
#endif

		return true;
	}

#endif // NB_WINDOWS

#ifdef NB_LINUX
	static inline Win * _FindWindow(WindowID _w)
	{
		WindowMap::iterator it = g_window_map.find(_w);
		return (it != g_window_map.end()) ? it->second : 0;
	}

	void Win_o::_WindowProc(XEvent & _event)
	{
		Win * p = _FindWindow(_event.xany.window);
		if(p)
		{
			p->WindowProc(_event);
		}
	}

	void Win::WindowProc(XEvent & _event)
	{
		// NB_PROFILE_INCLUDE;
		//if(_event.xany.window == _dummyWindow && _event.xany.type == SelectionRequest)
		//{
		//	_HandleSelectionRequest(_event);
		//	return;
		//}

		//E_TRACE_LINE("aaa");

		//E_TRACE_LINE("bbb");

		//E_ASSERT(this->o->hWnd == _event.xany.window);
		switch(_event.type)
		{
		case EnterNotify:
			////E_TRACE_LINE(L"EnterNotify: " + DebugObjectInfo(win));
			//{
			//	//float x =  _event.xcrossing.x;
			//	//float y =  _event.xcrossing.y;
			//	isOwnMouse = true;
			//	//OnMouseMove(x, y, false);
			//}
			break;
		case LeaveNotify:
			////E_TRACE_LINE(L"LeaveNotify: " + DebugObjectInfo(win));
			//isOwnMouse = false;
			//if(_grabMouseWin == win)
			//{
			//	_grabMouseWin = 0;
			//	g_grab_mouse_pane = 0;
			//}
			////this->OnMouseMove(-100, -100, true);
			break;
		case ConfigureNotify:
			{
				int x =  _event.xconfigure.x; // relate to parent
				int y =  _event.xconfigure.y; // relate to parent
				int w = _event.xconfigure.width;
				int h = _event.xconfigure.height;
				this->_OnSize(w, h);
			}
			break;
		case MapNotify:
			//OnVisibleChanged(true);
			break;
		case UnmapNotify:
			//OnVisibleChanged(false);
			break;
		case Expose:
			//PushClearAreaBuffer(this,Rect((float)_event.xexpose.x, (float)_event.xexpose.y, (float)_event.xexpose.width, (float)_event.xexpose.height));
			break;
		case ButtonPress:
			switch(_event.xbutton.button)
			{
			case Button1:
				{
					//unsigned long curLeftDownTime = Time::GetTicks();
					this->OnLeftDown(_event.xbutton.x, _event.xbutton.y);
					//if(curLeftDownTime > lastLeftDownTime + 50 && curLeftDownTime < lastLeftDownTime + 500
					//	&& abs(_event.xbutton.x -lastLeftDownX) + abs(_event.xbutton.y -lastLeftDownY) < 6 )
					//{
					//	this->OnLeftDoubleClick(_event.xbutton.x, _event.xbutton.y);
					//}
					//lastLeftDownTime = curLeftDownTime;
					//lastLeftDownX = _event.xbutton.x;
					//lastLeftDownY = _event.xbutton.y;
				}
				break;
			//case Button3:
			//	this->OnRightDown(_event.xbutton.x, _event.xbutton.y);
			//	break;
			//case Button4:
			//	this->OnMouseWheel(_event.xbutton.x, _event.xbutton.y, 1);
			//	break;
			case Button5:
				this->OnMouseWheel(_event.xbutton.x, _event.xbutton.y, -1);
				break;
			}
			break;
		case ButtonRelease:
			switch(_event.xbutton.button)
			{
			case Button1:
				this->OnLeftUp(_event.xbutton.x, _event.xbutton.y);
				break;
			//case Button3:
				//this->OnRightUp(_event.xbutton.x, _event.xbutton.y);
				//{
					//float x = _event.xbutton.x;
					//float y = _event.xbutton.y;
					//ClientToScreen(x, y);
					//this->ConvertCoord(x, y, Coord::ParentWin, Coord::ParentLayout);
					//this->OnContextMenu(x, y, false);
				//}
				//break;
				//case Button2:
				//	this->OnMiddleUp(_event.xbutton.x, _event.xbutton.y);
				//	break;
			}
			break;
		case MotionNotify:
			this->OnMouseMove(_event.xbutton.x,_event.xbutton.y);
			break;
		case KeyPress:
			//if(IsRealEnabled())
			//{
			//	KeySym keysym = 0;
			//	bool charsValid = false;
			//	bool keysymValid = false;
			//	string text;
			//	if(_xic)
			//	{
			//		int chars = 0;
			//		int buf_size = 64;
			//		wchar_t * buf = enew wchar_t[buf_size + 1];
			//		buf[0] = 0;
			//		bool again = true;
			//		while(again)
			//		{
			//			buf[0] = 0;
			//			keysym = 0;
			//			Status status;
			//			chars = XwcLookupString(_xic, (XKeyPressedEvent *)&_event.xkey, buf, buf_size, &keysym, &status);
			//			//E_TRACE_LINE("[nb] XwcLookupString()");
			//			switch(status)
			//			{
			//			case XBufferOverflow:
			//				//E_TRACE_LINE("[nb]     XBufferOverflow");
			//				buf_size+= 64;
			//				delete[] buf;
			//				buf = enew wchar_t[buf_size + 1];
			//				continue;
			//			case XLookupNone:
			//				//E_TRACE_LINE("[nb]     XLookupNone");
			//				again = false;
			//				break;
			//			case XLookupChars:
			//				//E_TRACE_LINE("[nb]     XLookupChars");
			//				charsValid = true;
			//				again = false;
			//				break;
			//			case XLookupKeySym:
			//				//E_TRACE_LINE("[nb]     XLookupKeySym");
			//				keysymValid = true;
			//				again = false;
			//				break;
			//			case XLookupBoth:
			//				//E_TRACE_LINE("[nb]     XLookupBoth");
			//				charsValid = true;
			//				keysymValid = true;
			//				again = false;
			//				break;
			//			}
			//		}

			//		buf[chars] = 0;
			//		text = string(buf);
			//		delete[] buf;
			//	}

			//	// xim dosn't work, try x11 method
			//	if(_xic == 0 || (!charsValid && !keysymValid))
			//	{
			//		//E_TRACE_LINE("[nb] XLookupString ");
			//		//keysym = XKeycodeToKeysym(_display, _event.xkey.keycode, 0);
			//		keysymValid = true;
			//		stringa buf1;
			//		buf1.reserve(64);
			//		//Status status;
			//		int len = XLookupString(&_event.xkey, &buf1[0], 64, &keysym, 0);
			//		buf1[len] = 0;
			//		text = string(buf1);
			//	}

				{
					KeySym keysym = XKeycodeToKeysym(_display, _event.xkey.keycode, 0);
					this->OnKeyDown(keysym);
				}

			//	// process chars
			//	if(charsValid)
			//	{
			//		for(int i = 0; i < text.length(); i++)
			//		{
			//			OnTextInput(text[i]);
			//		}
			//	}
			//}
			break;
		case KeyRelease:
			{
				KeySym keysym = XKeycodeToKeysym(_display, _event.xkey.keycode, 0);
				this->OnKeyUp(keysym);
			}
			break;
		//case FocusIn:
		//	if(!this->focus)
		//	{
		//		this->focus = true;
		//		this->OnFocusIn();
		//	}
		//	break;
		//case FocusOut:
		//	if(this->focus)
		//	{
		//		this->focus = false;
		//		this->OnFocusOut();
		//	}
		//	break;
		case DestroyNotify:
			E_ASSERT(0);
			break;
		//case SelectionRequest:
			//Clipboard::Singleton().o->HandleSelectionRequest(_event);
		//	break;
		//case SelectionNotify:
		//	Clipboard::Singleton().o->HandleSelectionNotify(_event);
		//	break;
		}
	}

	static void _HandleEvent(XEvent & _e)
	{
		// NB_PROFILE_INCLUDE;
		if(XFilterEvent(&_e, None))
		{
			return;
		}

		if(_e.type == ClientMessage)
		{
			XClientMessageEvent e1 = (XClientMessageEvent &)_e;
			if(e1.message_type == _atom_WM_PROTOCOLS && e1.format == 32
				&& Atom(e1.data.l[0]) == _atom_WM_DELETE_WINDOW)
			{
				if(Win::singleton)
				{
					Win::singleton->SafeDeleteThis();
				}
			}
			else if(Atom(e1.data.l[0]) == _atom_NET_WM_PING)
			{
				Win * win_o = _FindWindow(e1.window);
				if(win_o)
				{
					XEvent e2;
					e2.xclient.type         = ClientMessage;
					e2.xclient.display      = _display;
					e2.xclient.message_type = _atom_WM_PROTOCOLS;
					e2.xclient.format       = 32;
					e2.xclient.window       = ::XDefaultRootWindow(_display);
					e2.xclient.data.l[0]    = e1.data.l[0];
					e2.xclient.data.l[1]    = e1.data.l[1];
					e2.xclient.data.l[2]    = e1.data.l[2];
					e2.xclient.data.l[3]    = 0;
					e2.xclient.data.l[4]    = 0;
					XSendEvent(_display, e2.xclient.window, False, SubstructureRedirectMask|SubstructureNotifyMask, &e2);
				}
			}
			else
			{
				Win_o::_WindowProc(_e);
			}
		}
		else
		{
			Win_o::_WindowProc(_e);
		}
	}

	bool Win::Create(const WinFormat & _format, const Char * _unique_prop_name)
	{
		// NB_PROFILE_INCLUDE;
		if(o->hWnd)
		{
			return false;
		}
		o->format = _format;
		unsigned long valueMask = 0;
		XSetWindowAttributes attr;
		valueMask = CWBorderPixel|CWColormap|CWBitGravity;
		attr.border_pixel = 0;
		attr.colormap = _screenColormap;
		attr.bit_gravity = 0;

		int x, y, w, h;
		//x = _format.x;
		//y = _format.y;
		x = 0;
		y = 0;
		w = _format.w;
		h = _format.h;
		o->hWnd = ::XCreateWindow(
			_display,
			RootWindow(_display, _screenNum),
			x, y, w, h,
			0,
			0,
			InputOutput,
			0,
			valueMask,
			&attr
			);
		E_ASSERT(o->hWnd != 0);
		g_window_map[o->hWnd] = this;

		//XClassHint hint;
		//hint.res_name = (char*)"E_APPLICATION";
		//hint.res_class = (char*)"E_WIN";
		//XSetClassHint(_display, o->hWnd, &hint);

		::XSelectInput(_display, o->hWnd,
				ExposureMask
				| PointerMotionMask
				//| FocusChangeMask
				| ButtonMotionMask
				| Button1MotionMask
				//| Button2MotionMask
				| KeyPressMask
				| KeyReleaseMask
				| ButtonPressMask
				| ButtonReleaseMask
				| EnterWindowMask
				| LeaveWindowMask
				| StructureNotifyMask
				//| im_event_mask
				);

		XMapWindow(_display, o->hWnd);
		XSetWMProtocols (_display, o->hWnd, &_atom_WM_DELETE_WINDOW, 1);

		XSizeHints size_hints;
		size_hints.flags = PMinSize | PMaxSize;
		size_hints.min_width = w;
		size_hints.max_width = w;
		size_hints.min_height = h;
		size_hints.max_height = h;
		XSetWMNormalHints(_display, o->hWnd, &size_hints);

		//if(_CreateGraphics())
		//{
		//	return true;
		//}
		//else
		//{
		//	E_ASSERT(0);
		//	g_window_map.erase(o->hWnd);
		//	::XDestroyWindow(_display, o->hWnd);
		//	o->hWnd = 0;
		//	return false;
		//}
		return true;
	}

	Graphics * Win::CreateGraphics(const GraphicsFormat & _gf)
	{
		//// NB_PROFILE_INCLUDE;
		// TODO: _mode
		int visAttributes[20];
		int i = 0;
		visAttributes[i++] = GLX_USE_GL;
		visAttributes[i++] = GLX_USE_GL;
		visAttributes[i++] = GLX_RGBA;
		if(_gf.doubleBuffer)
		{
			visAttributes[i++] = GLX_DOUBLEBUFFER;
		}
		visAttributes[i++] = GLX_RED_SIZE;
		visAttributes[i++] = 8;
		visAttributes[i++] = GLX_GREEN_SIZE;
		visAttributes[i++] = 8;
		visAttributes[i++] = GLX_BLUE_SIZE;
		visAttributes[i++] = 8;
		visAttributes[i++] = GLX_ALPHA_SIZE;
		visAttributes[i++] = 8;
		if(false)
		{
			visAttributes[i++] = GLX_STENCIL_SIZE;
			visAttributes[i++] = 8;
		}
		visAttributes[i++] = None;

		//E_TRACE_LINE("++++++++++++++++++++++");
		XVisualInfo *visinfo = glXChooseVisual(_display, 0, visAttributes);
		E_ASSERT(visinfo);
		if(!visinfo)
		{
			return false;
		}
		//E_TRACE_LINE("----------------------");
		::GLXContext hGL = glXCreateContext(_display, visinfo, NULL, True);
		E_ASSERT(hGL);
		if(!hGL)
		{
			return false;
		}

		//g->imp->win = this;
		Graphics * & g = Graphics::singleton;
		g = enew Graphics();
		g->imp->win = this;
		g->imp->hWnd = o->hWnd;
		g->imp->hGL = hGL;
		g->imp->winW = o->format.w;
		g->imp->winH = o->format.h;
		g->imp->viewW = o->format.w;
		g->imp->viewH = o->format.h;
		g->imp->gf = _gf;
		g->_OnCreateDevice();
		_OnSize(o->format.w, o->format.h);
		return g;
	}


	bool Win::Resize(int _w, int _h, bool _fullScreen)
	{
		XWindowAttributes attr;
		::XGetWindowAttributes(_display, o->hWnd, &attr);
		int old_w = attr.width;
		int old_h = attr.height;
		if(old_w == _w && old_h == _h && o->format.fullScreen == _fullScreen)
		{
			return true;
		}

		o->format.w = _w;
		o->format.h = _h;
		o->format.fullScreen = _fullScreen;

		int x, y, w, h;
		x = o->format.x;
		y = o->format.y;
		w = o->format.w;
		h = o->format.h;
		//::XMoveWindow(_display, o->hWnd, x, y);
		::XResizeWindow(_display, o->hWnd, w, h);
		XSizeHints size_hints;
		size_hints.flags = PMinSize | PMaxSize;
		size_hints.min_width = w;
		size_hints.max_width = w;
		size_hints.min_height = h;
		size_hints.max_height = h;
		XSetWMNormalHints(_display, o->hWnd, &size_hints);
	}

#endif

	//void Win::OnRender()
	//{
	//}

	void Win::OnSize(int _w, int _h)
	{
	}

	void Win::_OnSize(int _w, int _h)
	{
		if(Graphics::singleton == 0)
		{
			return;
		}

		if(_w < 1)
		{
			_w = 1;
		}
		if(_h < 1)
		{
			_h = 1;
		}
		Graphics * g = Graphics::singleton;
		if(!g_is_opengl)
		{
#ifdef E_CFG_DIRECT3D
			// chnage size
			g->_OnLostDevice();
			UINT w1 = g->imp->d3dpp.BackBufferWidth;
			UINT h1 = g->imp->d3dpp.BackBufferHeight;
			g->imp->d3dpp.BackBufferWidth  = _w;
			g->imp->d3dpp.BackBufferHeight = _h;
			HRESULT hr = g->imp->device->Reset(&g->imp->d3dpp);
			E_ASSERT(hr != D3DERR_INVALIDCALL); // still has resource to be free.
			if(FAILED(hr))
			{
				E_ASSERT(0);
				g->imp->d3dpp.BackBufferWidth  = w1;
				g->imp->d3dpp.BackBufferHeight = h1;
			}
			g->_OnCreateDevice();
#endif
		}
		g->imp->winW = _w;
		g->imp->winH = _h;
		OnSize(_w, _h);
	}

	void Win::OnKeyDown(int _key)
	{
	}

	void Win::OnKeyUp(int _key)
	{
	}

	LoopState Win_o::PumpMessage()
	{
		// NB_PROFILE_INCLUDE;
		if(g_to_delete_win_singleton)
		{
			g_to_delete_win_singleton = false;
			E_ASSERT(Win::singleton != 0);
			Win::singleton->_Delete();
			E_ASSERT(Win::singleton == 0);
			Win::singleton = 0;
		}
		if(Win::singleton == 0)
		{
			return LoopQuit;
		}
#ifdef NB_WINDOWS
		MSG msg;
		if(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			E_ASSERT(msg.message != WM_QUIT);
			if(msg.message == WM_QUIT)
			{
				if(Win::singleton != 0)
				{
					Win::singleton->_Delete();
				}
				return LoopQuit;
			}
			else
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			return Win::singleton ? LoopBusy : LoopQuit;
		}
		else
		{
			return Win::singleton ? LoopFree : LoopQuit;
		}
#endif

#ifdef NB_LINUX
		XEvent evt;
		if(XEventsQueued(_display, QueuedAfterFlush ) > 0)
		{
			XNextEvent(_display, &evt);
			_HandleEvent(evt);
			return Win::singleton ? LoopBusy : LoopQuit;
		}
		else
		{
			return Win::singleton ? LoopFree : LoopQuit;
		}
#endif
	}




#ifdef NB_LINUX
#	ifdef UNICODE
	static void TextToXTextProperty(XTextProperty & _prop, const stringw & _text)
	{
		//Array<uint8> buf;
		//buf = _text.ToEncodeString(CharSets::GetDefault8());
		//stringa t(_text);
		wchar_t * p = const_cast<wchar_t*>((const wchar_t *)_text.c_str());
		XwcTextListToTextProperty(_display, &p, 1, XCompoundTextStyle, &_prop);
	}

	static void XTextPropertyToText(stringw & _text, const XTextProperty & _prop)
	{
		wchar_t ** p;
		int n;
		const XTextProperty * pProp = & _prop;
		XwcTextPropertyToTextList(_display, const_cast<XTextProperty *>(pProp), &p, &n);
		if(n > 0)
		{
			_text = string(p[0]);
		}
		else
		{
			_text = "";
		}
		XwcFreeStringList(p);
	}
#	else // UNICODE
	static void TextToXTextProperty(XTextProperty & _prop, const stringa & _text)
	{
		//Array<uint8> buf;
		//buf = _text.ToEncodeString(CharSets::GetDefault8());
		//stringa t(_text);
		char * p = const_cast<char*>((const char *)_text.c_str());
		XStringListToTextProperty(&p, 1, &_prop);
	}

	static void XTextPropertyToText(stringa & _text, const XTextProperty & _prop)
	{
		char ** p;
		int n;
		const XTextProperty * pProp = & _prop;
		XTextPropertyToStringList(const_cast<XTextProperty *>(pProp), &p, &n);
		if(n > 0)
		{
			_text = string(p[0]);
		}
		else
		{
			_text = "";
		}
		XFreeStringList(p);
	}
#	endif // UNICODE
#endif // NB_LINUX

	bool Win::SetTitle(const string & _title)
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		return ::SetWindowText(o->hWnd, _title.c_str()) ? true : false;
#endif

#ifdef NB_LINUX
		if(o->hWnd)
		{
			XTextProperty windowName;
			TextToXTextProperty(windowName, _title);
			XSetWMName(_display, o->hWnd, &windowName);
			XFree(windowName.value);
			return true;
		}
		else
		{
			return false;
		}
#endif
	}

	bool Win::IsActive() const
	{
#ifdef NB_WINDOWS
		return ::GetActiveWindow() == o->hWnd;
#endif
#ifdef NB_LINUX
		return true;
#endif
	}

	bool Win::IsVisible() const
	{
#ifdef NB_WINDOWS
		DWORD style = ::GetWindowLong(o->hWnd, GWL_STYLE);
		return (style & WS_MINIMIZE) == 0 && (style & WS_VISIBLE) != 0;
#endif
#ifdef NB_LINUX
		return true;
#endif
	}

//	bool Win::IsMinimize() const
//	{
//#ifdef NB_WINDOWS
//		return (::GetWindowLong(o->hWnd, GWL_STYLE) & WS_MINIMIZE) != 0;
//#endif
//#ifdef NB_LINUX
//		return false;
//#endif
//	}

	bool Win::GetSize(int & _w, int & _h) const
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		RECT rect;
		if(::GetClientRect(o->hWnd, &rect))
		{
			_w = rect.right - rect.left;
			_h = rect.bottom - rect.top;
			if(_w < 1)
			{
				_w = 1;
			}
			if(_h < 1)
			{
				_h = 1;
			}
			return true;
		}
		else
		{
			_w = 1;
			_h = 1;
			return false;
		}
#endif

#ifdef NB_LINUX
		if(o->hWnd)
		{
			XWindowAttributes attr;
			::XGetWindowAttributes(_display, o->hWnd, &attr);
			_w = attr.width;
			_h = attr.height;
			return true;
		}
		else
		{
			_w = 1;
			_h = 1;
			return false;
		}
#endif
	}

	bool Win::GetPosition(int & _x, int & _y) const
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		RECT rect;
		if(::GetWindowRect(o->hWnd, &rect))
		{
			_x = rect.left;
			_y = rect.top;
			return true;
		}
		else
		{
			_x = 100;
			_y = 100;
			return false;
		}
#endif

#ifdef NB_LINUX
		if(o->hWnd)
		{
			XWindowAttributes attr;
			::XGetWindowAttributes(_display, o->hWnd, &attr);
			_x = attr.x;
			_y = attr.y;
			return true;
		}
		else
		{
			_x = 100;
			_y = 100;
			return false;
		}
#endif
	}




//	bool Win::SetClientSize(int _w, int _h)
//	{
//		// NB_PROFILE_INCLUDE;
//#ifdef NB_WINDOWS
//		int x = 100;
//		int y = 100;
//		RECT rect = {x, y, x+_w, y+_h};
//		::AdjustWindowRect(&rect, ::GetWindowLong(o->hWnd, GWL_STYLE), FALSE);
//		//_x = (float)rect.left;
//		//_y = (float)rect.top;
//		_w = rect.right - rect.left;
//		_h = rect.bottom - rect.top;
//
//		return ::SetWindowPos(o->hWnd, 0, 0, 0, _w, _h, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE) ? true : false;
//#endif
//#ifdef NB_LINUX
//		::XResizeWindow(_display, o->hWnd, _w, _h);
//		return true;
//#endif
//	}
	void Win_o::RenderBlackWin()
	{
		// NB_PROFILE_INCLUDE;
		if(hWnd == 0)
		{
			return;
		}
#ifdef NB_WINDOWS
		HDC hDCTemp = ::GetDC(hWnd);
		RECT rect;
		::GetClientRect(hWnd, &rect);
		::FillRect(hDCTemp, &rect, (HBRUSH)::GetStockObject(BLACK_BRUSH));
		::ReleaseDC(hWnd, hDCTemp);
#endif
	}
	void Win::SafeDeleteThis()
	{
		E_ASSERT(this == singleton);
		g_to_delete_win_singleton = true;
	}
	void Win::OnRealTime(bool _busy)
	{
		E_ASSERT(0);
		if(!_busy)
		{
			e::Sleep(10);
		}
	}
	LoopState Win::PumpMessage(double _timeLimit)
	{
		double t1 = Time::GetTicks() + _timeLimit;
		LoopState loopState;
		do
		{
			loopState = Win_o::PumpMessage();
		}while(LoopBusy == loopState && Time::GetTicks() < t1);
#ifdef NB_CFG_VERBOSE
		if(Time::GetTicks() >= t1)
		{
			E_TRACE_LINE("[nb] (WW) Win::PumpMessage(): busy.");
		}
#endif
		return loopState;
	}

	void Win::OnLeftDown(float _x, float _y)
	{
		if(Pane::rootPane == 0)
		{
			return;
		}
		Pane * p = g_grab_mouse_pane
			? g_grab_mouse_pane
			: g_pane_stack->Find(_x, _y);
		if(p == 0)
		{
			return;
		}
		p->WinToPane(_x, _y);
		p->OnLeftDown(_x , _y);
	}

	void Win::OnLeftUp(float _x, float _y)
	{
		if(Pane::rootPane == 0)
		{
			return;
		}
		Pane * p = g_grab_mouse_pane
			? g_grab_mouse_pane
			: g_pane_stack->Find(_x, _y);
		if(p == 0)
		{
			return;
		}
		p->WinToPane(_x, _y);
		p->OnLeftUp(_x , _y);
	}

	void Win::OnMouseMove(float _x, float _y)
	{
		if(Pane::rootPane == 0)
		{
			return;
		}

		Pane * p;
		if(g_grab_mouse_pane)
		{
			p = g_grab_mouse_pane;
			float x0 = 0;
			float y0 = 0;
			p->PaneToWin(x0, y0);
			float x1 = x0 + p->W() - 1;
			float y1 = y0 + p->H() - 1;

			bool hover = _x >= x0 && _x <= x1 && _y >= y0 && _y <= y1;
			g_mouse_hover_pane = hover ? p : 0;
			//if(hover && g_mouse_hover_pane != p)
			//{
			//	o->oldHover = g_mouse_hover_pane;
			//	g_mouse_hover_pane = p;
			//	o->hoverFrac = 0;
			//	//if(o->oldHover)
			//	//{
			//	//	o->oldHover->OnHoverChanged(o->oldHover, p);
			//	//}
			//	//p->OnHoverChanged(o->oldHover, p);
			//	//UpdateCursor();
			//}
			//else if(!hover && g_mouse_hover_pane == p)
			//{
			//	o->oldHover = g_mouse_hover_pane;
			//	g_mouse_hover_pane = 0;
			//	o->hoverFrac = 0;
			//	//p->OnHoverChanged(p,  g_mouse_hover_pane);
			//	// UpdateCursor();
			//}
		}
		else
		{
			p = g_pane_stack->Find(_x, _y);
			g_mouse_hover_pane = p;
			//bool hover = p != 0;
			//if(hover && g_mouse_hover_pane != p)
			//{
			//	o->oldHover = g_mouse_hover_pane;
			//	g_mouse_hover_pane = p;
			//	o->hoverFrac = 0;
			//	//if(o->oldHover)
			//	//{
			//	//	o->oldHover->OnHoverChanged(o->oldHover, p);
			//	//}
			//	//p->OnHoverChanged(o->oldHover, p);
			//	//UpdateCursor();
			//}
			//else if(!hover && g_mouse_hover_pane != 0)
			//{
			//	o->oldHover = g_mouse_hover_pane;
			//	g_mouse_hover_pane = 0;
			//	o->hoverFrac = 0;
			//	//o->oldHover->OnHoverChanged(o->oldHover,  g_mouse_hover_pane);
			//	// UpdateCursor();
			//}
		}
		if(p)
		{
			p->WinToPane(_x, _y);
			p->OnMouseMove(_x , _y);
		}
	}

	void Win::OnMouseWheel(float _x, float _y, float _dz)
	{
		if(Pane::rootPane == 0)
		{
			return;
		}
		Pane * p = g_grab_mouse_pane
			? g_grab_mouse_pane
			: g_pane_stack->Find(_x, _y);
		if(p == 0)
		{
			return;
		}
		p->WinToPane(_x, _y);
		p->OnMouseWheel(_x, _y, _dz);
	}

	void Win::_Delete()
	{
		delete this;
	}

	bool Win::OpenUrl(const string & _url)
	{
		E_TRACE_LINE("[e_ui] Win::OpenUrl() : " + _url);
#ifdef NB_WINDOWS
		return (HINSTANCE)32 < ShellExecute(o->hWnd, L"open", _url.c_str(),L"",L"", SW_SHOW);
#endif

#ifdef NB_LINUX
		stringa cmd = "xdg-open \'" + stringa(_url) + "\'"; // note: xdg-open may require perl-uri
		return 0 == system(cmd.c_str());
#endif
	}

	//Graphics * Win::GetGraphics()
	//{
	//	return Graphics::singleton;
	//}

	void Win::SetCursorVisible(bool _visible)
	{
#ifdef NB_WINDOWS
		::ShowCursor(_visible ? TRUE : FALSE);
#endif
	}


	//void Win_o::PushGrabMousePane(Pane * _w)
	//{
	//	if(g_grab_mouse_pane != _w)
	//	{
	//		g_grab_mouse_pane = _w;
	//		g_grab_mouse_pane_stack.push_back(_w);
	//	}
	//}


	void Win::DrawUI()
	{
		if(Pane::rootPane == 0)
		{
			return;
		}
//		PaneStack * ps = g_pane_stack;
		g_pane_stack->Validate();
		Graphics * & g = Graphics::singleton;
		g->Enable(GS_SCISSOR_TEST);
		for(size_t i=0; i<g_pane_stack->stack.size(); i++)
		{
			Pane * p = g_pane_stack->stack[i];
			g->SetScissor((int)p->imp->box.L(), (int)p->imp->box.T(), (int)p->imp->box.R(), (int)p->imp->box.B());
			g->TranslateMatrix(p->imp->offset.x, p->imp->offset.y);
			p->Draw();
			g->TranslateMatrix(-p->imp->offset.x, -p->imp->offset.y);
		}
		g->Disable(GS_SCISSOR_TEST);
	}

	void Win::Step()
	{
		if(Pane::rootPane == 0 || !Pane::rootPane->visible)
		{
			return;
		}
		g_pane_stack->Validate();
		for(size_t i=0; i<g_pane_stack->stack.size(); i++)
		{
			g_pane_stack->stack[i]->Step();
		}
	}

}
