
// #include "../config.h"
#include <z_kxx/ui/main_menu.h>

namespace e
{
	UIBase::UIBase(KxxWin * _win)
		: win(_win)
	{
		ui_x0 = 0;
		ui_y0 = 0;
		ui_x1 = 100;
		ui_y1 = 100;
	}

	UIBase::~UIBase()
	{
	}	
	
	bool UIBase::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		return false;
	}

	void UIBase::RenderBack()
	{
	}

	void UIBase::RenderFront()
	{
	}

	void UIBase::Step()
	{
	}

	void UIBase::SetBoundRect(float _x0, float _y0, float _x1, float _y1)
	{
		ui_x0 = _x0;
		ui_y0 = _y0;
		ui_x1 = _x1;
		ui_y1 = _y1;
	}
}
