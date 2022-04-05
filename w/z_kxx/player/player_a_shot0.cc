
// #include "../config.h"
#include <z_kxx/player/player_a_shot0.h>
#include <z_kxx/sprite/spark1.h>
#include <z_kxx/player/player_a.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/ani_spark.h>

namespace e
{
	static const int PLAYER_SHOT0_TRANSPARENT_TIMER = K_LOGIC_FPS / 3;
	// E_DEFINE_MEMORY_POOL(PlayerAShot0);
	PlayerAShot0::PlayerAShot0(PlayerA * _player)
	{
		player = _player;
		tex = _player->shot0;
		this->transparent_timer = PLAYER_SHOT0_TRANSPARENT_TIMER;
		hue = 1;
	}


	PlayerAShot0::~PlayerAShot0()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	//void PlayerAShot0::Render()
	//{
	//}


	void PlayerAShot0::OnCrash(const Vector2 & _pt)
	{
		float angleDelta = (rand() % 21 - 10) * 0.3f;

		AniSpark * p = enew AniSpark(player->shot0Spark, ang - 0.5f * PI, ang-angleDelta - 0.5f * PI, 1.0f, 1.0f);
		p->pos = pos;
		p->blm = BM_ADD;
	//	p->speed = speed * 0.5f * 1;
		float speed_angle = speed.Angle()  - angleDelta;
		float speed_value = speed.length() * 0.5f  * 1;
		p->speed.SetPolar(speed_value, speed.Angle());
		kxxwin->AddSparkToList(p, RL_GAME_PLAYER_PET);			
		kxxwin->AddSmallSpark(_pt, 3, hue);
		ActivateSputter();
	}

	bool PlayerAShot0::Step()
	{
		pos.x+= speed.x;
		pos.y+= speed.y;
		if(transparent_timer)
		{
			--transparent_timer;
			clr.a = (1.0f - 0.7f * float(transparent_timer) / float(PLAYER_SHOT0_TRANSPARENT_TIMER));
		}
		return pos.y > -30;
	}


}
