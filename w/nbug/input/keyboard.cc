#ifdef NB_WINDOWS
#	include <windows.h>
#endif

#ifdef NB_LINUX
#   include <X11/Xlib.h>
#	include <string.h>
#endif

#include <nbug/input/keyboard.h>
#include <nbug/core/debug.h>

namespace e
{
#ifdef NB_LINUX
	extern Display * _display;
	extern int _screenNum;
	extern Colormap _screenColormap;
#endif
	Keyboard::Keyboard()
	{
		memset(keyboardMap, 0, sizeof(keyboardMap));
	}

	Keyboard::~Keyboard()
	{
	}

	bool Keyboard::Map(uint _keySymbol, uint _state)
	{
		// NB_PROFILE_INCLUDE;
		// find first bit
		int n = 0;
		while(((_state >> n) & 0x00000001) == 0)
		{
			n++;
		}

		if(n > 31)
		{
			return false;
		}
#ifdef NB_WINDOWS
		keyboardMap[n] = _keySymbol;
#endif

#ifdef NB_LINUX
		keyboardMap[n] = ::XKeysymToKeycode(_display, _keySymbol);
#endif
		return true;
	}

	void Keyboard::Poll(uint & _state)
	{
		// NB_PROFILE_INCLUDE;
		_state = 0;
#ifdef NB_WINDOWS
		for(int i=0; i<32; i++)
		{
			if(keyboardMap[i] && (GetAsyncKeyState(keyboardMap[i]) & 0x8000)!= 0)
			{
				_state|= 0x00000001 << i;
			}
		}
#endif
#ifdef NB_LINUX
		char keys_return[32];
		::XQueryKeymap(_display, keys_return);
		for(int i=0; i<32; i++)
		{
			if(keyboardMap[i] && (keys_return[keyboardMap[i]/8] & (1 << (keyboardMap[i]%8))) != 0)
			{
				_state|= 0x00000001 << i;
			}
		}
#endif
	}

	int Keyboard::CharToKey(Char _ch)
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		switch(_ch)
		{
		case L'\r':
			return Keyboard::Enter;
		case L' ':
			return Keyboard::Space;
		case L'\t':
			return Keyboard::Tab;
		}
		return _ch;
#endif

#ifdef NB_LINUX
		return XKeycodeToKeysym(_display, _ch, 0);
#endif
	}

	Char Keyboard::KeyToChar(int _key)
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		switch(_key)
		{
		case Keyboard::Enter:
			return L'\r';
		case Keyboard::Space:
			return L' ';
		case Keyboard::Tab:
			return L'\t';
		}
		return _key;
#endif

#ifdef NB_LINUX
		return XKeysymToKeycode(_display, _key);
#endif
	}

	bool Keyboard::IsCtrlDown()
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		return (GetKeyState(VK_CONTROL) & 0x8000) != 0;
#endif

#ifdef NB_LINUX
		KeyCode a = ::XKeysymToKeycode(_display, XK_Control_L);
		KeyCode b = ::XKeysymToKeycode(_display, XK_Control_R);
		char keys_return[32];
		::XQueryKeymap(_display, keys_return);
		bool ret = (keys_return[a/8] & (1 << (a%8))) != 0 || (keys_return[b/8] & (1 << (b%8))) != 0;
		//E_TRACE_LINE("[nb] Keyboard::IsCtrlDown(): ret = " + string(ret));
		return ret;
#endif
	}

	bool Keyboard::IsAltDown()
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		return (GetKeyState(VK_MENU) & 0x8000) != 0;
#endif
#ifdef NB_LINUX
		KeyCode a = ::XKeysymToKeycode(_display, XK_Alt_L);
		KeyCode b = ::XKeysymToKeycode(_display, XK_Alt_R);
		char keys_return[32];
		::XQueryKeymap(_display, keys_return);
		bool ret = (keys_return[a/8] & (1 << (a%8))) != 0 || (keys_return[b/8] & (1 << (b%8))) != 0;
		//E_TRACE_LINE("[nb] Keyboard::IsAltDown(): ret = " + string(ret));
		return ret;
#endif
	}

	bool Keyboard::IsShiftDown()
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_WINDOWS
		return (GetKeyState(VK_SHIFT) & 0x8000) != 0;
#endif

#ifdef NB_LINUX
		KeyCode a = ::XKeysymToKeycode(_display, XK_Shift_L);
		KeyCode b = ::XKeysymToKeycode(_display, XK_Shift_R);
		char keys_return[32];
		::XQueryKeymap(_display, keys_return);
		bool ret = (keys_return[a/8] & (1 << (a%8))) != 0 || (keys_return[b/8] & (1 << (b%8))) != 0;
		//E_TRACE_LINE("[nb] Keyboard::IsShiftDown(): ret = " + string(ret));
		return ret;
#endif
	}
}
