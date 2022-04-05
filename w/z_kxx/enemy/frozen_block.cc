
// #include "../config.h"
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/enemy/frozen_block.h>

namespace e
{
	FrozenBlock::FrozenBlock(PlayerShot * _playerShot)
	{
		init_life = life = 10;
		init_timer = timer = 2 * K_LOGIC_FPS;
		active_time = timer - 10;
		pos_offset.x = -20 + logic_random_float() * 40;
		pos_offset.y = -20 + logic_random_float() * 40;
		playerShot = _playerShot;
		pos = _playerShot->pos - pos_offset;
		hsz.x = 32;
		hsz.y = 32;
		dr = 0.4f;
		init_clr = _playerShot->clr;
		clr.a = 0;
		ang = LogicRandomAngle();
		tex = kxxwin->frozenTex;
		active = false;
	}


	FrozenBlock::~FrozenBlock()
	{
		delete playerShot;
	}

	bool FrozenBlock::Step()
	{
		Enemy::Step();
		//playerShot->pos = pos;
		if(timer)
		{
			timer--;
			if(timer == active_time)
			{
				active = true;
			}
			float f = float(timer) / init_timer;
			playerShot->clr.a = init_clr.a * f;
			this->clr.a = CalcFadeInFadeOut(init_timer, timer, 0.3f);
		}
		return timer != 0;
	}

	void FrozenBlock::Render()
	{
		playerShot->Render();
		Enemy::Render();
	}
	
	//void FrozenBlock::Damage(float _damage, bool _ignore_defence)
	//{
	//}

}

