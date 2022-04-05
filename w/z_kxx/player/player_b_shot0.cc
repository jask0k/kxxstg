
// #include "../config.h"
#include <z_kxx/player/player_b_shot0.h>
#include <z_kxx/sprite/spark1.h>
//#include <z_kxx/player/player_a.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player_b.h>
#include <z_kxx/sprite/ani_spark.h>

namespace e
{
	// E_DEFINE_MEMORY_POOL(PlayerBShot0);
	PlayerBShot0::PlayerBShot0(PlayerB * _player)
	{
		player = _player;
		tex = _player->shot0;
		hue = 2;
	}


	PlayerBShot0::~PlayerBShot0()
	{

	}


	void PlayerBShot0::OnCrash(const Vector2 & _pt)
	{
		AniSpark * p = enew AniSpark(player->shot0Spark, ang, ang, 1.0f, 1.0f, 1.0f, 0.0f);
		p->pos = pos;
		p->blm = BM_ADD;
		p->speed = speed * 0.5f * 1;
		kxxwin->AddSparkToList(p, RL_GAME_PLAYER_PET);			
		kxxwin->AddSmallSpark(_pt, 3, hue);
		ActivateSputter();
	}

	bool PlayerBShot0::Step()
	{
		pos.x+= speed.x;
		pos.y+= speed.y;
		return pos.y > -30;
	}


}
