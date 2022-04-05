
#pragma once

#include <z_kxx/ui/ui_base.h>

namespace e
{
	class OptionsMenu : public UIBase
	{
		static const float spacing;
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_left_w;
		float menu_right_w;
		float item_h;
		void CalcMenuPos();
		bool require_restart;
	public:
		TexRef bgTex;
		string title;
		struct Item : public Object
		{
			int activeOption;
			string text;
			StringArray options;
			Callback callback;
			bool require_restart;
			bool IsButton() const
			{
				return options.empty();
			}
			Item()
			{
				activeOption = 0;
				require_restart = false;
			}
		};
		Array<Item*> items;
		int activeItem;
		OptionsMenu(KxxWin * _win);
		~OptionsMenu();
		void RenderBack() override;
		void RenderFront() override;
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA disabledFontColor;
		RGBA titleColor;
		bool OnJoystickDown(int _joystick, int _button) override;
	};
}

