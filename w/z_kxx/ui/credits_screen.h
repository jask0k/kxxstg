
#pragma once
#include <nbug/core/callback.h>
#include <nbug/gl/tex.h>
#include <z_kxx/ui/ui_base.h>

namespace e
{
	class CreditsScreen : public UIBase
	{
	public:
		TexRef bgTex;
		uint32 timer;
		CreditsScreen(KxxWin * _win);
		~CreditsScreen();
		void RenderBack() override;
		void RenderFront() override;
		bool OnJoystickDown(int _joystick, int _button) override;
		Callback callback;
	};
}
