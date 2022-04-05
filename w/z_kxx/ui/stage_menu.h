
#pragma once

#include <z_kxx/ui/ui_base.h>

namespace e
{
	class StageMenu : public UIBase
	{
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_w;
		float item_h;
		void CalcMenuPos();
		bool isInGame;
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
		StageMenu(KxxWin * _win, bool _isInGame);
		~StageMenu();
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
		void Enable(int _stage, bool _enable)
		{
			E_ASSERT(_stage >= 0 && _stage < (int)items.size() - 1);
			items[_stage]->enabled = _enable;
		}
	};
}

