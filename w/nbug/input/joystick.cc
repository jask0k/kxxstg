
//#include "private.h"

#ifdef E_CFG_JOYSTICK
#	ifdef NB_WINDOWS
#		define DIRECTINPUT_VERSION  0x0800
#		include <dinput.h>
#	endif
#	ifdef NB_LINUX
#		include <string.h>
#		include <sys/stat.h>
#		include <fcntl.h>
#		include <unistd.h>
#		include <errno.h>
#		include <linux/joystick.h>
#	endif
#endif // E_CFG_JOYSTICK

#include <nbug/tl/array.h>
#include <nbug/input/joystick.h>
#include <nbug/core/debug.h>
#include <nbug/core/dll_load.h>

#ifdef E_CFG_JOYSTICK

namespace e
{
	static int g_joystick_count = 0;

#	ifdef NB_WINDOWS
	static void * g_direct8_dll = 0;
	static LPDIRECTINPUT8 g_directInput = 0;
	typedef HRESULT (WINAPI *FUNC_DirectInput8Create)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter);
	static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
	{
		Array<DIDEVICEINSTANCE> * p = (Array<DIDEVICEINSTANCE>*) pvRef;
		p->push_back(*lpddi);
		return DIENUM_CONTINUE;
	}

	struct MYJSDATA 
	{ 
		LONG x;
		LONG y;
		BYTE btn0;
		BYTE btn1;
		BYTE btn2;
		BYTE btn3;
		BYTE btn4;
		BYTE btn5;
		BYTE btn6;
		BYTE btn7;
	}; 

	static GUID myGUID_XAxis = {0xA36D02E0,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00}};
	static GUID myGUID_YAxis = {0xA36D02E1,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00}};
	static GUID myGUID_Button = {0xA36D02F0,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00}};

	static DIOBJECTDATAFORMAT my_rgodf[] = 
	{ 
	  { &myGUID_XAxis, FIELD_OFFSET(MYJSDATA, x),
		DIDFT_AXIS | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_YAxis, FIELD_OFFSET(MYJSDATA, y), 
		DIDFT_AXIS | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn0),
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn1), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn2), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn3), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn4),
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn5), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn6), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	  { &myGUID_Button, FIELD_OFFSET(MYJSDATA, btn7), 
		DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0, }, 
	}; 
    
	DIDATAFORMAT my_df = 
	{ 
		sizeof(DIDATAFORMAT),
		sizeof(DIOBJECTDATAFORMAT),
		DIDF_ABSAXIS,
		sizeof(MYJSDATA),
		(sizeof(my_rgodf) / sizeof(DIOBJECTDATAFORMAT)), 
		my_rgodf,
	}; 

#	endif

	Joystick::Joystick()
	{
#	ifdef NB_WINDOWS
		native = 0;
		if(g_directInput == 0)
		{
			E_ASSERT(g_direct8_dll == 0);
			g_direct8_dll = hex_dll_open("dinput8.dll");
			if(g_direct8_dll)
			{
				FUNC_DirectInput8Create pDirectInput8Create = (FUNC_DirectInput8Create) hex_dll_get_symbol(g_direct8_dll, "DirectInput8Create");

				if(pDirectInput8Create)
				{
					GUID myIID_IDirectInput8 = {0xBF798031,0x483A,0x4DA2,{0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00} };
					HRESULT hr = pDirectInput8Create(GetModuleHandle(NULL),
						DIRECTINPUT_VERSION,
						myIID_IDirectInput8,
						(VOID**)&g_directInput,
						NULL);
					if(FAILED(hr))
					{
						g_directInput = 0;
					}
				}
			}
			if(g_directInput == 0)
			{
				message(L"[nb] (WW) Failed to create DirectInput object.");
			}
		}
#	endif

#	ifdef NB_LINUX
		native = -1;
#	endif
		memset(table, 0, sizeof(table));
		g_joystick_count++;
	}

	Joystick::~Joystick()
	{
		Close();
		g_joystick_count--;
		if(g_joystick_count == 0)
		{
#	ifdef NB_WINDOWS
			if(g_directInput)
			{
				g_directInput->Release();
				g_directInput = 0;
			}

			if(g_direct8_dll)
			{
				hex_dll_close(g_direct8_dll);
				g_direct8_dll = 0;
			}
#	endif
		}
	}

	bool Joystick::_Open(uint _id)
	{
#	ifdef NB_WINDOWS
		if(native)
		{
			Close();
		}
		Array<DIDEVICEINSTANCE> allDevice;
		g_directInput->EnumDevices(DI8DEVCLASS_GAMECTRL ,
			 &DIEnumDevicesCallback,
			 &allDevice,
			 DIEDFL_ALLDEVICES);

		if(allDevice.empty())
		{
			return false;
		}

#		ifdef NB_DEBUG
		message(L"[nb] Joystick list:");
		for(uint i=0; i<allDevice.size(); i++)
		{
			DIDEVICEINSTANCE & d = allDevice[i];
			message(L"\t" + string(d.tszInstanceName));
		}
#		endif // NB_DEBUG
		if(_id < 0 || _id >= allDevice.size())
		{
			_id = 0;
		}
		DIDEVICEINSTANCE & d = allDevice[_id];
        if (FAILED(g_directInput->CreateDevice(d.guidInstance, &native, NULL)))
		{
            return false;
        }

		if(FAILED(native->SetDataFormat(&my_df)))
		{
			message(L"[kx] (WW) Failed to set joystick format");
			return false;
		}

		DIPROPRANGE propRange;
		propRange.diph.dwSize       = sizeof(DIPROPRANGE);
		propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propRange.diph.dwHow        = DIPH_BYOFFSET;//DIPH_BYID;
		propRange.lMin              = -1000;
		propRange.lMax              = +1000;

		propRange.diph.dwObj        = FIELD_OFFSET(MYJSDATA, x);
		if (FAILED(native->SetProperty(DIPROP_RANGE, &propRange.diph)))
		{
			return false;
		}

		propRange.diph.dwObj        = FIELD_OFFSET(MYJSDATA, y);
		if (FAILED(native->SetProperty(DIPROP_RANGE, &propRange.diph)))
		{
			return false;
		}

		return true;
#	else // NB_WINDOWS
		if(native >= 0)
		{
			Close();
		}
		state = 0;
		stringa name = "/dev/input/js" + string(_id);
		native = open(name.c_str(), O_NONBLOCK);
		return native != -1;
#	endif // NB_WINDOWS
	}

	bool Joystick::OpenDefault()
	{
		return _Open(0);
	}

	bool Joystick::Ready() const
	{
#	ifdef NB_WINDOWS
		return native != 0;
#	endif

#	ifdef NB_LINUX
		return native >= 0;
#	endif
	}

	void Joystick::Close()
	{
		if(native)
		{
#	ifdef NB_WINDOWS
			native->Release();
#	else
			close(native);
#	endif
		}
		native = 0;
	}

	bool Joystick::Map(Button _btn, uint _state)
	{
		// NB_PROFILE_INCLUDE;
		if(_btn < 0 || _btn >= _BTN_MAX)
		{
			return false;
		}
		table[_btn] = _state;
		return true;
	}

	void Joystick::Poll(uint & _state)
	{
		_state = 0;

		if(native == 0)
		{
			return;
		}

#	ifdef NB_WINDOWS
		HRESULT hr;
		// Poll the device to read the current state
		hr = native->Poll();
		if (FAILED(hr))
		{
			hr = native->Acquire();
			while (hr == DIERR_INPUTLOST)
			{
				hr = native->Acquire();
			}

			if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
			{
				return;
			}

			if (hr == DIERR_OTHERAPPHASPRIO)
			{
				return;
			}
		}

		MYJSDATA js;
		if (FAILED(hr = native->GetDeviceState(sizeof(MYJSDATA), &js)))
		{
			return;
		}

		if(js.x < -500)
		{
			//E_TRACE_LINE(L"left");
			_state |= table[JS_BTN_L];
		}
		else if(js.x > 500)
		{
			//E_TRACE_LINE(L"right");
			_state |= table[JS_BTN_R];
		}
		if(js.y < -500)
		{
			//E_TRACE_LINE(L"up");
			_state |= table[JS_BTN_U];
		}
		else if(js.y > 500)
		{
			//E_TRACE_LINE(L"down");
			_state |= table[JS_BTN_D];
		}

		if(js.btn0)
		{
			//E_TRACE_LINE(string(0) + L" pushed.");
			_state |= table[JS_BTN_0];
		}

		if(js.btn1)
		{
			//E_TRACE_LINE(string(1) + L" pushed.");
			_state |= table[JS_BTN_1];
		}

		if(js.btn2)
		{
			//E_TRACE_LINE(string(2) + L" pushed.");
			_state |= table[JS_BTN_2];
		}

		if(js.btn3)
		{
			//E_TRACE_LINE(string(3) + L" pushed.");
			_state |= table[JS_BTN_3];
		}

		if(js.btn4)
		{
			//E_TRACE_LINE(string(0) + L" pushed.");
			_state |= table[JS_BTN_4];
		}

		if(js.btn5)
		{
			//E_TRACE_LINE(string(1) + L" pushed.");
			_state |= table[JS_BTN_5];
		}

		if(js.btn6)
		{
			//E_TRACE_LINE(string(2) + L" pushed.");
			_state |= table[JS_BTN_6];
		}

		if(js.btn7)
		{
			//E_TRACE_LINE(string(3) + L" pushed.");
			_state |= table[JS_BTN_7];
		}
#	else // NB_WINDOWS
		js_event ev;
		while(read(native, &ev, sizeof(js_event)) > 0)
		{
			switch(ev.type)
			{
			case JS_EVENT_AXIS:
				if(ev.number == 0)
				{
					// X
					if(ev.value < 0)
					{
						state &= ~table[JS_BTN_R];
						state |= table[JS_BTN_L];
					}
					else if(ev.value > 0)
					{
						state &= ~table[JS_BTN_L];
						state |= table[JS_BTN_R];
					}
					else
					{
						state &= ~(table[JS_BTN_L] | table[JS_BTN_R]);
					}
				}
				else if(ev.number == 1)
				{
					// Y
					if(ev.value < 0)
					{
						state &= ~table[JS_BTN_D];
						state |= table[JS_BTN_U];
					}
					else if(ev.value > 0)
					{
						state &= ~table[JS_BTN_U];
						state |= table[JS_BTN_D];
					}
					else
					{
						state &= ~(table[JS_BTN_U] | table[JS_BTN_D]);
					}
				}
				break;
			case JS_EVENT_BUTTON:
				if(ev.number >= JS_BTN_0 || ev.number <= JS_BTN_3)
				{
					if(ev.value)
					{
						state |= table[ev.number];
					}
					else
					{
						state &= ~table[ev.number];
					}
				}
				break;
			}
		}
	   	if(errno != EAGAIN)
		{
	       // ERROR
			Close();
	   	}

		_state = state;
#	endif // NB_WINDOWS
	}
}

#else //E_CFG_JOYSTICK

namespace e
{
	Joystick::Joystick()
	{}
	Joystick::~Joystick()
	{}
	bool Joystick::OpenDefault()
	{ return false; }
	bool Joystick::Ready() const
	{ return false; }
	void Joystick::Close()
	{}
	void Joystick::Poll(uint & _state)
	{}
	bool Joystick::Map(Button _btn, uint _state)
	{ return false; }
	bool Joystick::_Open(uint _id)
	{ return false; }
}
#endif // E_CFG_JOYSTICK

