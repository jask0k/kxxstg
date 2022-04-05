
// #include "../config.h"
#include <z_kxx/sprite/avatar.h>
#include <z_kxx/util/util_func.h>

namespace e
{

#define HUGEBODY_TIMER_MAX (3 * K_LOGIC_FPS)

	void Avatar::CreateBoss(TexRef _tex)
	{
		tex = _tex;
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;
		speed.x = 0;
		speed.y = 100.0f / K_LOGIC_FPS;
		timer = HUGEBODY_TIMER_MAX;

		pos.x = K_GAME_W - tex->W() * 0.4f;
		pos.y = 0;
		clr.a = 0;
	}

	void Avatar::CreatePlayer(TexRef _tex)
	{
		tex = _tex;
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;
		speed.x = 0;
		speed.y = -100.0f / K_LOGIC_FPS;
		timer = HUGEBODY_TIMER_MAX;
		pos.x = tex->W() * 0.4f;
		pos.y = K_GAME_H;
		clr.a = 0;
	}


	bool Avatar::Step()
	{
		if(timer)
		{
			timer--;
			pos.x+= speed.x;
			pos.y+= speed.y;
			clr.a = 0.7f * CalcFadeInFadeOut(HUGEBODY_TIMER_MAX, timer, 0.3f);
		}
		return timer != 0;
	}

}
