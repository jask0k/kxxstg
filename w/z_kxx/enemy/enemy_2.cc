
// #include "../config.h"
#include <z_kxx/enemy/enemy_2.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	Enemy2::Enemy2(uint _type, uint _index, uint _total)
		: type(_type)
		, index(_index)
		, total(_total)
	{
		init_life = life = 3.2;
		// def = 0.3f;

		SetSize(40);

		if(_index %3 == 0)
		{
			ani5 = kxxwin->fairyRedAni5;
			drops.Set(0, DROP_SMALL_POWER, 1, 1);
		}
		else
		{
			ani5 = kxxwin->fairyBlueAni5;
			drops.Set(0, DROP_SMALL_POINT, 1, 1);
		}

		Vector2 v;
		{
			float xr = K_GAME_W;
			float left = (K_GAME_W - xr) / 2;
			float dx = xr / (total - 1);

			pos.y = -20;

			float a0 = - 0.5f * PI;
			float da;
			Vector2 c;
			c.y = K_GAME_H * 0.6f;
			float rx = K_GAME_W;
			float ry = c.y - 100;
			if(type == 0)
			{
				c.x = 0;
				da = 0.5f * PI;
				pos.x = left + dx * index;
			}
			else
			{
				c.x = K_GAME_W;
				da = -0.5f * PI;
				pos.x = K_GAME_W - (left + dx * index);
			}

			da/= 16.0f;
			float a = a0 + da*(index + 2);
			v.x = c.x + rx * cos(a);
			v.y = c.y + ry * sin(a);
		}



		state = 0;
		timer = S2F(0.5f);

		// s = 0.5 * a * t ^ 2;
		// a = 2 * s / t ^ 2;
		Vector2 s = v - pos;
		acc = s * 2 / float(timer) / float(timer);
		vel = acc * float(timer);
	}


	Enemy2::~Enemy2()
	{
	}

	bool Enemy2::Step()
	{
		Enemy::Step();

		ani5.Step((int)(vel.x*100));
		tex = ani5.GetTex();

		switch(state)
		{
		case 0:
			if(--timer==0)
			{
				state = 1;
				timer = S2F(2);
				AddEnemyShot(pos, ES_SNIPE_0);
			}
			else
			{
				pos+=vel;
				vel-=acc;
			}
			return true;
		case 1:
			if(--timer==0)
			{
				state = 2;
				timer = S2F(0.5f);
				//AddEnemyShot(pos, ES_SNIPE_0);				
				acc = acc * 0.2f;
			}
			return true;
		case 2:
			pos+=vel;
			vel+=acc;
			return !IsOutOfGameArea(pos, 42);
		}
		return false;
	}


	Enemy2Maker::Enemy2Maker(uint _type)
		: RepeatEnemyMaker(10, 0.2f)
	{
		type = _type;
		LastDropGroup * g = enew LastDropGroup();
		g->drops.allRandomPos = true;
		g->drops.Set(0, DROP_SMALL_POWER, 1, 1);
		g->drops.Set(1, DROP_SMALL_POINT, 2, 2);
		group = g;
	}

	Enemy * Enemy2Maker::OnMakeOne()
	{
		Enemy2 * p = enew Enemy2(type, GetCurrent(), GetTotal());
		return p;
	}

}

