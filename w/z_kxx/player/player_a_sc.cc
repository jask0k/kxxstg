
// #include "../config.h"
#include <z_kxx/player/player_a_sc.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	PlayerAYYYSC::PlayerAYYYSC(const Vector2 & _pt, TexRef _tex)
	{
		fragile = false;
		power = 20.0f;
		collisionFrac.x = 0.8f;
		collisionFrac.y = 0.8f;

		tex = _tex;
		timer = SPELL_CARD_DURATION;
		float t = (float) timer;

		pos = _pt;
		hsz.x = hsz.y = 192;
		scl.y = scl.x = 0.3f;
		clr.a = 1.0f;
		
		scale_delta = (1.0f - scl.x) / t;
		speed_y     = PS2PF(-50);
		angle_delta = PS2PF(PI * 2);
		alpha_delta = (0.2f - 1.0f) / t;
	}


	PlayerAYYYSC::~PlayerAYYYSC()
	{
	}

	bool PlayerAYYYSC::Step()
	{
		if(timer)
		{
			unavailableTimer++;
			if(unavailableTimer > K_LOGIC_FPS_MUL * 12)
			{
				unavailableTimer = 0;
			}
		
			timer--;
			scl.x+= scale_delta;
			scl.y+= scale_delta;
			pos.y+= speed_y;
			ang+= angle_delta;
			clr.a+= alpha_delta;
			return true;
		}
		else
		{
			return false;
		}
	}

	void PlayerAYYYSC::OnCrash(const Vector2 & _pt)
	{
		kxxwin->AddSmallSpark(_pt, 3, hue);
		kxxwin->AddFlashSpark(_pt.x, _pt.y, hue);
	}
}

