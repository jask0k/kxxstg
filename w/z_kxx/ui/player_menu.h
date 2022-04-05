
#pragma once

#include <z_kxx/ui/ui_base.h>

namespace e
{
	class StaticImage;
	class DynamicImage;
	class PlayerMenu : public UIBase
	{
		float menu_x0;
		float menu_y0;
		float menu_h;
		float menu_w;
		float item_h;
		void CalcMenuPos();
		int activeItem;
		stringa name;
		void SwitchToPlayer(int _i);
		TexRef level_tex[K_LEVEL_COUNT];
		TexRef ex_stage_tex;
	public:
		int level;
		StaticImage * bg;
		string title;
		struct Item
		{
			int state;
			uint32 timer;
			DynamicImage * body;
			string text;
			bool enabled;
			Callback callback;
		};
		Array<Item*> items;
		PlayerMenu(KxxWin * _win);
		~PlayerMenu();
		void RenderBack() override;
		void RenderFront() override;
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA titleColor;
		RGBA disabledFontColor;
		bool OnJoystickDown(int _joystick, int _button) override;
		int GetActiveItem()
		{ return activeItem; }
		void Init(int _activeItem);
	};
}

