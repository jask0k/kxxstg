#include <nbug/gl/graphics.h>
// #include "../config.h"
#include <z_kxx/ui/ending_screen.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	EndingScreen::EndingScreen(KxxWin * _win)
		: UIBase(_win)
	{
		timer = 0;
		bgTex = kxxwin->LoadTex("ending");
	}

	EndingScreen::~EndingScreen()
	{
		//if(bgTex)
		//{
		//	bgTex->Release();
		//}
	}

	void EndingScreen::RenderBack()
	{
	}

	void EndingScreen::RenderFront()
	{
		timer++;
		// graphics->SetTexMode(TextureMode::replace);
		// graphics->BlendOff();
		graphics->SetColor(1, 1, 1, 1);
		graphics->SetTexMode(TM_MODULATE);
		graphics->BindTex(bgTex);
		//graphics->SetTextureFilter(false);
		graphics->DrawQuad(0, 0, K_VIEW_W, K_VIEW_H);
		//graphics->SetTextureFilter(true);
		// graphics->BlendOn();
	}

	bool EndingScreen::OnJoystickDown(int _joystick, int _button)
	{
		if((int)timer > 1*60 && _joystick == 0 && _button == MAPED_BUTTON_FIRE)
		{
			callback(this);
			return true;
		}
		return false;
	}
}
