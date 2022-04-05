
// #include "../config.h"
#include <z_kxx/player/player_a_shot1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player_a.h>
#include <z_kxx/sprite/ani_spark.h>

namespace e
{
	static const float PLAYER_SHOT1_SPEED = 700.f / K_LOGIC_FPS;
	static const int PLAYER_SHOT1_TRANSPARENT_TIMER = K_LOGIC_FPS / 5;
	static const int PLAYER_SHOT1_GUIDE_TIMER = K_LOGIC_FPS / 10;
	PlayerAShot1::PlayerAShot1(PlayerA * _player, const Vector2 & _pos, float _ang)
	{
		pos = _pos;
		player = _player;
		tex = player->shot1;
		this->transparent_timer = PLAYER_SHOT1_TRANSPARENT_TIMER;
		this->guide_timer = PLAYER_SHOT1_GUIDE_TIMER;
	/*	if(_enemy)
		{
			_enemy->AddFollower(this);
			ang = (_enemy->pos - pos).Angle();
		}
		else
		{
			ang = -0.5f * PI;
		}*/
		ang = _ang;
		vel.x = PLAYER_SHOT1_SPEED * cos(ang);
		vel.y = PLAYER_SHOT1_SPEED * sin(ang);
		hsz.y = 10;
		hsz.x = 10;
		hue = 1;
	}


	PlayerAShot1::~PlayerAShot1()
	{
	}

	void PlayerAShot1::OnCrash(const Vector2 & _pt)
	{
		float angleDelta = (rand() % 21 - 10) * 0.1f;

		AniSpark * p = enew AniSpark(player->shot1Spark, ang, angleDelta * 10, 1.0f, 2.0f);
		p->pos = pos;
		p->blm = BM_ADD;
	//	p->speed = speed * 0.5f * 1;
		float speed_angle = ang - angleDelta;
		float speed_value = PLAYER_SHOT1_SPEED * 0.3f;
		p->speed.SetPolar(speed_value, speed_angle);
		kxxwin->AddSparkToList(p, RL_GAME_PLAYER_PET);			
		kxxwin->AddSmallSpark(_pt, 3, hue);
		ActivateSputter();
	}

	bool PlayerAShot1::Step()
	{
		if(followed)
		{
			if(--guide_timer == 0)
			{
				guide_timer = PLAYER_SHOT1_GUIDE_TIMER;
				float target_ang = (followed->pos - this->pos).Angle();
				float da = target_ang - ang;
				da = angle_normalize(da);
				da = angle_limit(da, 0.1f);
				ang+= da;
				vel.x = PLAYER_SHOT1_SPEED * cos(ang);
				vel.y = PLAYER_SHOT1_SPEED * sin(ang);
			}
		}
		if(transparent_timer)
		{
			--transparent_timer;
			clr.a = (1.0f - float(transparent_timer) / float(PLAYER_SHOT1_TRANSPARENT_TIMER));
		}
		pos+= vel;
		return !IsOutOfGameArea(pos, 40);
	}

}
