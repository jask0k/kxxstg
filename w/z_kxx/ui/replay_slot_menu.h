
#pragma once

#include <z_kxx/ui/ui_base.h>

namespace e
{
	class ReplaySlotMenu : public UIBase
	{
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_w;
		float item_h;
		void CalcMenuPos();
		bool save;
	public:
		TexRef bgTex;
		string title;
		struct Item
		{
			string text;
			bool enabled;
			Callback callback;
		};
		Array<Item*> items;
		int activeItem;
		ReplaySlotMenu(KxxWin * _win, bool _save);
		~ReplaySlotMenu();
		void RenderBack() override;
		void RenderFront() override;
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA disabledFontColor;
		RGBA titleColor;
		bool OnJoystickDown(int _joystick, int _button) override;
		void Init(const Callback & _callback);
		Path GetSelectSlotPath();
	};
}

