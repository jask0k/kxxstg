
#pragma once

#include <z_kxx/ui/ui_base.h>
#include <nbug/tl/array.h>
#include <nbug/core/callback.h>
#include <nbug/core/def.h>
#include <nbug/gl/tex.h>
namespace e
{
	class Dandelions;
	class StaticImage;
	class DynamicImage;
	class MainMenu : public UIBase
	{
		float item_h;
		float item_w;
		float text_w;
		float text_h;
		StaticImage * bg;
		Dandelions * dandelions;
		DynamicImage * title;
		uint32 fadeInTimer;
		TexRef bodyTex;
		uint32 joystickIdleTimer;
		TexRef btn1Tex;
		TexRef btn2Tex;
	public:
		struct Item
		{
			uint step; // 0 - wait, 1 - enter, 2 - back, 3 - stop
			uint timer;
			float speed;
			Rect rect;
			string text;
			bool enabled;
			Callback callback;
		};
		Array<Item*> items;
		int activeItem;
		MainMenu(KxxWin * _win);
		~MainMenu();
		void CalcMenuSize();
		FontRef fontFace;
		void SetBoundRect(float _x0, float _y0, float _x1, float _y1) override;
		RGBA defaultFontColor;
		RGBA disabledFontColor;
		void RenderBack() override;
		void RenderFront() override;
		void Step() override;
		bool OnJoystickDown(int _joystick, int _button) override;
	};
}

