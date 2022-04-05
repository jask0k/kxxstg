
#pragma once

#include <nbug/core/obj.h>
#include <nbug/gl/tex.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/color.h>
#include <nbug/gl/graphics.h>
#include <nbug/core/callback.h>
#include <nbug/gl/rect.h>
#include <z_kxx/globals.h>

namespace e
{
	class KxxWin;
	class UIBase : public Object
	{
	protected:
		float ui_x0;
		float ui_y0;
		float ui_x1;
		float ui_y1;
		KxxWin * win;
	public:
		UIBase(KxxWin * _win);
		virtual ~UIBase();
		virtual void SetBoundRect(float _x0, float _y0, float _x1, float _y1);
		virtual bool OnJoystickDown(int _joystick, int _button);
		virtual void RenderBack();
		virtual void RenderFront();
		virtual void Step();
	};
}
