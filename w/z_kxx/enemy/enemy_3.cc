
// #include "../config.h"
#include <z_kxx/enemy/enemy_3.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	
	Enemy3::Enemy3(uint _type, uint _index, uint _total)
		: type(_type%4)
		, index(_index)
		, total(_total)
	{
		init_life = life = 3.2;

		SetSize(40);

		if(_index == 0)
		{
			ani5 = kxxwin->fairyRedAni5;
			drops.Set(0, DROP_SMALL_POWER, 1, 1);
			hue = 1;
		}
		else
		{
			ani5 = kxxwin->fairyBlueAni5;
			drops.Set(0, DROP_SMALL_POINT, 1, 1);
			hue = 2;
		}

		vel_f = PS2PF(170);
		timer2 = S2F(type < 2 ? 2.0f : 3.0f);
		float al = type < 2 ? 1.2f : 0.5f;
		da = 0.5f * PI / float(timer2*al);
		Vector2 v;
		{
			switch(type)
			{
			case 0:
				pos.x = 120;
				pos.y = -20;
				v.y = 150;
				vel.y = vel_f;
				a = 0.5f * PI;
				da= -da;
				break;
			case 1:
				pos.x = K_GAME_W - 120;
				pos.y = -20;
				v.y = 150;
				vel.y = vel_f;
				a = 0.5f * PI;
				break;
			case 2:
				pos.x = 50;
				pos.y = K_GAME_H + 20;
				v.y = 200;
				vel.y = -vel_f;
				a = -0.5f * PI;
				break;
			case 3:
				pos.x = K_GAME_W - 50;
				pos.y = K_GAME_H + 20;
				v.y = 200;
				vel.y = -vel_f;
				a = -0.5f * PI;
				da= -da;
				break;
			}
			v.x = pos.x;
			vel.x = 0;
		}


//		acc.x = 0;
//		acc.y = 0;
		state = 0;
		timer = int((v.y - pos.y) / vel.y);

		shot_timer = type < 2 ? S2F(0.6f) : S2F(1.7f);
		// s = 0.5 * a * t ^ 2;
		// a = 2 * s / t ^ 2;
		//Vector2 s = v - pos;
		//acc = s * 2 / float(timer) / float(timer);
		//vel = acc * float(timer);
	}


	Enemy3::~Enemy3()
	{
	}

	bool Enemy3::Step()
	{
		Enemy::Step();

		ani5.Step((int)(vel.x*100));
		tex = ani5.GetTex();

		if(--shot_timer == 0)
		{
			shot_timer = S2F(4.0f);
			AddEnemyShot(pos, ES_SNIPE_0);
		}
		switch(state)
		{
		case 0:
			if(--timer==0)
			{
				state = 1;
				timer = timer2;
			}
			else
			{
				pos+=vel;
			}
			return true;
		case 1:
			if(--timer==0)
			{
				state = 2;
			}
			else
			{
				pos+=vel;
				a+= da;
				vel.x = vel_f * cos(a);
				vel.y = vel_f * sin(a);
			}
			return true;
		case 2:
			pos+= vel;
			return !IsOutOfGameArea(pos, 42);
		}
		return false;
	}


	Enemy3Maker::Enemy3Maker(uint _type)
		: RepeatEnemyMaker(10, 0.2f)
	{
		type = _type;
		LastDropGroup * g = enew LastDropGroup();
		g->drops.allRandomPos = true;
		g->drops.Set(0, DROP_SMALL_POWER, 1, 1);
		g->drops.Set(1, DROP_SMALL_POINT, 2, 2);
		group = g;
	}

	Enemy * Enemy3Maker::OnMakeOne()
	{
		Enemy3 * p = enew Enemy3(type, GetCurrent(), GetTotal());
		return p;
	}

}

