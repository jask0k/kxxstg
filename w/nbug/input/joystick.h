
#pragma once
#include <nbug/core/def.h>

struct IDirectInputDevice8;
namespace e
{
	class Joystick
	{
	public:
		enum Button
		{
			JS_BTN_L,
			JS_BTN_R,
			JS_BTN_U,
			JS_BTN_D,
			JS_BTN_0, // ps triangle
			JS_BTN_1, // ps circle
			JS_BTN_2, // ps cross
			JS_BTN_3, // ps square
			JS_BTN_4, // ps triangle
			JS_BTN_5, // ps circle
			JS_BTN_6, // ps cross
			JS_BTN_7, // ps square
			_BTN_MAX,
		};

		Joystick();
		~Joystick();
		bool OpenDefault();
		void Close();
		void Poll(uint & _state);
		bool Map(Button _btn, uint _state);
		bool Ready() const;
	private:
#ifdef NB_WINDOWS
		IDirectInputDevice8 * native;
#endif

#ifdef NB_LINUX
		int native;
		uint state;
#endif
		uint table[_BTN_MAX];
		bool _Open(uint _id);
	};
}

