
// #include "../config.h"
#include <z_kxx/enemy/enemy_1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
#define INIT_LIFE 10
#define FREQ0 S2F(1.0f)
#define DY 0.012f
	Enemy1::Enemy1(bool _leftToRight, float _x, float _y)
		: leftToRight(_leftToRight)
	{
		pos.x = _x;
		pos.y = _y;
		ani = _leftToRight ? kxxwin->kedamaRightAni : kxxwin->kedamaLeftAni;
		//speed.y = PS2PF(100);
		init_life = life =  INIT_LIFE;
		drops.Set(0, DROP_SMALL_POINT, 1, 1);
		SetSize(40);
		state = 0;
		stateTimer = FREQ0;
		dy = 0;
	}


	Enemy1::~Enemy1()
	{
	}

	bool Enemy1::Step()
	{
		Enemy::Step();
		if(ani)
		{
			ani.Step();
			tex = ani.GetTex();
		}

		E_ASSERT(stateTimer > 0);
		stateTimer--;
		if(stateTimer == 0)
		{
			switch(state)
			{
			case 0:
				// fly horizontal
				{
					bool flag = false;
					if(life < INIT_LIFE)
					{
						// may change behaver after been hurt
						if(logic_random_float() < 0.2f)
						{
							state = 1;
							ani = leftToRight ? kxxwin->kedamaPonderRightAni : kxxwin->kedamaPonderLeftAni;
							stateTimer = ani.GetTotalSpan();
							tex = ani.GetTex();
							flag = true;
						}
					}

					if(!flag)
					{
						//AddEnemyShot(pos, 9);
						AddEnemyShot(pos, ES_KEDAMA_SNIPE);
						stateTimer = FREQ0;
					}
				}
				break;
			case 1:
				{
					// wander
					state = 2;
					ani.Detach();
					tex =  leftToRight ? kxxwin->kedamaFall0Right : kxxwin->kedamaFall0Left;
					stateTimer = S2F(0.8f);
				}
				break;
			case 2:
				{
					// fall
					state = 3;
					tex =  leftToRight ? kxxwin->kedamaFall1Right : kxxwin->kedamaFall1Left;
					stateTimer = 0x7fffffff;
					AddEnemyShot(pos, ES_SHOTGUN);
				}
				break;
			default:
				E_ASSERT(0);
			}
		}

		switch(state)
		{
		case 0:
			// fly horizontal
			pos.x += leftToRight ? PS2PF(100) : -PS2PF(100);
			break;
		case 1:
			break;
		case 2:
		default:
			pos.x += leftToRight ? PS2PF(80) : -PS2PF(80);
			pos.y += dy;
			dy+= DY / (K_LOGIC_FPS_MUL*K_LOGIC_FPS_MUL);
			break;
		}
		return !IsOutOfGameArea(pos, 42);
	}

	Enemy1Maker::Enemy1Maker()
		: RepeatEnemyMaker(4, 0.2f)
	{
		left = logic_random_int() % 2 ? true : false;
		LastDropGroup * g = enew LastDropGroup();
		g->drops.allRandomPos = true;
		g->drops.Set(0, DROP_SMALL_POINT, 1, 1);
		g->drops.Set(1, DROP_SMALL_POINT, 2, 3);
		group = g;
	}

	Enemy * Enemy1Maker::OnMakeOne()
	{
		float x = left ? -20.0f : K_GAME_W + 20.0f;
		Enemy1 * p = enew Enemy1(left, x, 50 + logic_random_float() * 150);
		return p;
	}

}

