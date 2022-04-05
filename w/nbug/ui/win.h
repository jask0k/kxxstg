
#pragma once

#include <nbug/core/obj.h>
#include <nbug/gl/image.h>
#include <nbug/gl/tex.h>
#include <nbug/gl/fbo.h>
#include <nbug/gl/color.h>


//struct IDirect3DVertexBuffer9;
union _XEvent;

namespace e
{

	enum LoopState
	{
		LoopQuit,
		LoopBusy,
		LoopFree,
	};

	struct WinFormat
	{
		int x, y, w, h;
		bool allowResize   : 1;
		bool maximized     : 1;
		bool fullScreen    : 1;
		WinFormat();
	};

	struct GraphicsFormat
	{
		bool vsync         : 1;
		bool doubleBuffer : 1;
		int zBufferBits; // 24   
		int alphaBufferBits; // 8
		int stencilBufferBits; // 8
		GraphicsFormat();
	};

	class Pane;
	struct Win_o;
	//class Pane;
	//class FontOjbect;
	struct Vector2;
	struct RGBA;
	//class FontImp;
	class Callback;
	class Graphics;
	class Win : public Object
	{
		friend class Pane;
		friend struct Win_o;
		Win_o * o;
		void _Delete();
		void _OnSize(int _w, int _h);
	public:
		Win();
		bool Create(const WinFormat & _format, const Char * _unique_prop_name = 0);
		Graphics * CreateGraphics(const GraphicsFormat & _gf);
		bool SetTitle(const string & _title);
		bool IsActive() const;
		bool IsVisible() const;
		bool GetSize(int & _w, int & _h) const;
		bool Resize(int _w, int _h, bool _fullScreen = false);
		bool GetPosition(int & _x, int & _y) const;
		void SafeDeleteThis();
		static LoopState PumpMessage(double _timeLimit);
		void SetCursorVisible(bool _visible);
		bool OpenUrl(const string & _url);
		void DrawUI();
		void Step();
		static Win * singleton;
	protected:
		virtual ~Win(); // use SafeDeleteThis instead.
		virtual void OnRealTime(bool _busy);
		virtual void OnSize(int _w, int _h);
		virtual void OnKeyDown(int _key);
		virtual void OnKeyUp(int _key);
		virtual void OnLeftDown(float _x, float _y);
		virtual void OnLeftUp(float _x, float _y);
		//virtual void OnRightDown(float _x, float _y);
		//virtual void OnRightUp(float _x, float _y);
		virtual void OnMouseMove(float _x, float _y);
		virtual void OnMouseWheel(float _x, float _y, float _dz);
#ifdef NB_WINDOWS
		int WindowProc(uint _msg, uintx _wParam, intx _lParam);
#endif

#ifdef NB_LINUX
		void WindowProc(_XEvent & _event);
#endif
	};
}
