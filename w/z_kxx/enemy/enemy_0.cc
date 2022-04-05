
// #include "../config.h"
#include <z_kxx/enemy/enemy_0.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/shot/petal_shot.h>

namespace e
{
	Enemy0::Enemy0(int _type, float _x, float _y)
		: type(_type)
	{
		pos.x = _x;
		pos.y = _y;
		ani5 = _type == 0 ? kxxwin->fairyBlueAni5 : kxxwin->fairyRedAni5;
		speed.y = PS2PF(100);
		init_life = life =  10;
		def = 0.5f;
		drops.Set(0, DROP_SMALL_POINT, 1, 1);
		SetSize(40);

		prob[sStand] = 2;
		prob[sForward] = 1;
		prob[sShot] = 1;
		prob[sEscape] = 0;

		SetState(sForward);
	}


	Enemy0::~Enemy0()
	{
	}

	bool Enemy0::Step()
	{
		Enemy::Step();

		ani5.Step((int)(speed.x*100));
		tex = ani5.GetTex();


		pos.x+= speed.x;
		pos.y+= speed.y;
		E_ASSERT(stateTimer > 0);
		stateTimer--;
		if(stateTimer == 0)
		{
			SetState((State)GetRandomState(prob, _STATE_MAX));
		}
		return !IsOutOfGameArea(pos, 42);
	}

	void Enemy0::SetState(State _s)
	{
		stateTimer = S2F(1);
		state = _s;
		switch(state)
		{
		case sStand:
			speed.x = 0;
			speed.y = 0;
			break;
		case sForward:
			// speed.x = 0;
			// speed.y = PS2PF(100);
			speed = (kxxwin->player->pos - pos).GetNormal() * PS2PF(100);
			break;
		case sShot:
			speed.x = 0;
			speed.y = 0;
			{
				//RepeatShooter * p;
				//p = enew RepeatShooter(this);
				//p->RegisterShotS(0.5f, ES_COMET, 0.5f, 1);
				//AddLogicAction(p);
		
				float angles[100];
				int n = kxxwin->state.level * 3 + 8;
				float angle = (kxxwin->player->pos - pos).Angle();
				CalcCircleShot(angles, n, angle);
				for(int i=0; i<n; i++)
				{
					PetalShot * shot = enew PetalShot(pos.x, pos.y, angles[i]);
					kxxwin->AddEnemyShotToList(shot);
				}
				kxxwin->PlaySE("enemy-shot-0", pos, 0.5f);
				AddEnemyShot(pos, ES_COMET);
			}
			break;
		case sEscape:
			break;
		}
	}

	Enemy0Maker::Enemy0Maker()
		: RepeatEnemyMaker(4, 0.2f)
	{
		LastDropGroup * g = enew LastDropGroup();
		g->drops.allRandomPos = true;
		g->drops.Set(0, DROP_SMALL_POINT, 1, 1);
		g->drops.Set(1, DROP_SMALL_POINT, 2, 2);
		group = g;
	}

	Enemy * Enemy0Maker::OnMakeOne()
	{
		Enemy0 * p = enew Enemy0(logic_random_int() % 2, 40 + logic_random_float() * 400, -20);
		return p;
	}

}

