
#ifndef E_KEYBOARD_H
#define E_KEYBOARD_H

#ifdef NB_LINUX
#	include <X11/keysym.h>
#endif

#include <nbug/core/def.h>

namespace e
{
	class Keyboard
	{
	public:
		//static bool IsControlChar(Char _ch);
		static int CharToKey(Char _ch);
		static Char KeyToChar(int _key);
		enum
		{
	#ifdef NB_WINDOWS
			Backspace = 0x08,
			Tab = 0x09,
			Enter = 0x0D,
			Shift  = 0x10,
			Control = 0x11,
			Alt =  0x12,
			Pause = 0x13,
			Esc = 0x1B,
			Space = 0x20,
			PageUp = 0x21,
			PageDown = 0x22,
			End = 0x23,
			Home = 0x24,
			Left = 0x25,
			Up = 0x26,
			Right = 0x27,
			Down = 0x28,
			insert = 0x2D,
			Delete = 0x2E,
			F1 = 0x70,
			F2 = 0x71,
			F3 = 0x72,
			F4 = 0x73,
			F5 = 0x74,
			F6 = 0x75,
			F7 = 0x76,
			F8 = 0x77,
			F9 = 0x78,
			F10 = 0x79,
			F11 = 0x7A,
			F12 = 0x7B,

			LeftShift  = 0xA0,
			RightShift  = 0xA1,
			LeftControl = 0xA2,
			RightControl = 0xA3,
			//#define VK_LSHIFT         0xA0
			//#define VK_RSHIFT         0xA1
			//#define VK_LCONTROL       0xA2
			//#define VK_RCONTROL       0xA3
			//#define VK_LMENU          0xA4
			//#define VK_RMENU          0xA5
			W = 'W',
			A = 'A',
			S = 'S',
			D = 'D',
			J = 'J',
			K = 'K',
			L = 'L',
			P = 'P',
			Z = 'Z',
			X = 'X',
			w = 'W',
			a = 'A',
			s = 'S',
			d = 'D',
			j = 'J',
			k = 'K',
			l = 'L',
			p = 'P',
			z = 'Z',
			x = 'X',
			N1 = '1',
			N2 = '2',
			N3 = '3',
			N4 = '4',
			N5 = '5',
			N6 = '6',
			N7 = '7',
			N8 = '8',

	#endif //NB_WINDOWS

	#ifdef NB_LINUX
			Backspace = XK_BackSpace,
			Tab = XK_Tab,
			Enter = XK_Return,
			LeftShift  = XK_Shift_L,
			RightShift  = XK_Shift_R,
			LeftControl = XK_Control_L,
			RightControl = XK_Control_R,
			LeftAlt =  XK_Alt_L,
			RightAlt =  XK_Alt_R,
			Pause = XK_Pause,
			Esc = XK_Escape,
			Space = XK_space,
			PageUp = XK_Page_Up,
			PageDown = XK_Page_Down,
			End = XK_End,
			Home = XK_Home,
			Left = XK_Left,
			Up = XK_Up,
			Right = XK_Right,
			Down = XK_Down,
			insert = XK_Insert, 
			Delete = XK_Delete,
			F1 = XK_F1,// F1 key
			F2 = XK_F2,// F2 key
			F3 = XK_F3,// F3 key
			F4 = XK_F4,// F4 key
			F5 = XK_F5,// F5 key
			F6 = XK_F6,// F6 key
			F7 = XK_F7,// F7 key
			F8 = XK_F8,// F8 key
			F9 = XK_F9,// F9 key
			F10 = XK_F10,// F10 key
			F11 = XK_F11,// F11 key
			F12 = XK_F12,// F12 key
			F13 = XK_F13,// F13 key
			F14 = XK_F14,// F14 key
			F15 = XK_F15,// F15 key
			F16 = XK_F16,// F16 key
			F17 = XK_F17,//H F17 key
			F18 = XK_F18,//H F18 key
			F19 = XK_F19,//H F19 key
			F20 = XK_F20,//H F20 key
			F21 = XK_F21,//H F21 key
			F22 = XK_F22,//H F22 key
			F23 = XK_F23,//H F23 key
			F24 = XK_F24,//H F24 key
			W = XK_w,
			A = XK_a,
			S = XK_s,
			D = XK_d,
			J = XK_j,
			K = XK_k,
			L = XK_l,
			P = XK_p,
			Z = XK_z,
			X = XK_x,

			N1 = XK_1,
			N2 = XK_2,
			N3 = XK_3,
			N4 = XK_4,
			N5 = XK_5,
			N6 = XK_6,
			N7 = XK_7,
			N8 = XK_8,
	#endif
		};

		static bool IsCtrlDown();
		static bool IsAltDown();
		static bool IsShiftDown();
		//static bool IsCapsLock
		Keyboard();
		~Keyboard();
		bool Map(uint _keySymbol, uint _state);
		void Poll(uint & _state);
	private:
		uint keyboardMap[32];
	};
}

#endif
