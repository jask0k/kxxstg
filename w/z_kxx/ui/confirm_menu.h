
#pragma once

#include <z_kxx/ui/ui_base.h>
#include <nbug/tl/array.h>
#include <nbug/core/callback.h>
#include <nbug/gl/font/font.h>
#include <nbug/gl/color.h>

namespace e
{
	class ConfirmMenu : public UIBase
	{
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_w;
		float item_h;
		void CalcMenuPos();
	public:
		string title;
		struct Item
		{
			string text;
			bool enabled;
			Callback callback;
		};
		Array<Item*> items;
		int activeItem;
		ConfirmMenu(KxxWin * _win);
		~ConfirmMenu();
		void RenderBack() override;
		void RenderFront() override;
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA titleColor;
		bool OnJoystickDown(int _joystick, int _button) override;
	};
}

