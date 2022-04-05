
#pragma once

#include <z_kxx/ui/ui_base.h>

namespace e
{
	class ConfirmMenu;
	class PauseMenu : public UIBase
	{
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_w;
		float item_h;
		void CalcMenuPos();
		ConfirmMenu * confirmMenu;
	public:
		string title;
	//	GLuint tex;
		struct Item
		{
			string text;
			bool enabled;
			bool needConfirm;
			Callback callback;
			Item()
			{
				enabled = false;
				needConfirm = false;
			}
		};
		Array<Item*> items;
		int activeItem;
		PauseMenu(KxxWin * _win);
		~PauseMenu();
		void RenderBack() override;
		void RenderFront() override;
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA disabledFontColor;
		RGBA titleColor;
		bool OnJoystickDown(int _joystick, int _button) override;
		void ShowConfirmMenu();
		int OnConfirmYes(void * _p);
		int OnConfirmNo(void * _p);
	};
}

