
#include <nbug/gl/graphics.h>
// #include "../config.h"
#include <z_kxx/ui/credits_screen.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	CreditsScreen::CreditsScreen(KxxWin * _win)
		: UIBase(_win)
	{
		timer = 0;
		bgTex = kxxwin->LoadTex("credits");
	}

	CreditsScreen::~CreditsScreen()
	{
		//if(bgTex)
		//{
		//	bgTex->Release();
		//}
	}

	void CreditsScreen::RenderBack()
	{
	}

	void CreditsScreen::RenderFront()
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

	bool CreditsScreen::OnJoystickDown(int _joystick, int _button)
	{
		if((int)timer > 1*60 && _joystick == 0 && _button == MAPED_BUTTON_FIRE)
		{
			callback(this);
			return true;
		}
		return false;
	}
}
