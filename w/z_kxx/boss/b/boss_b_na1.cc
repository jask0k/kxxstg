#include <z_kxx/boss/b/boss_b_na1.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	class BossBNA1Shot : public EnemyShot
	{
		int state;
		float acc;
		float vel;
		uint32 timer;
	public:
		Ani ani;
		BossBNA1Shot(uint32 _wait)
		{
			ani = kxxwin->LoadAni(L"boss-b-na1-shot");
			tex = ani.GetTex();
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;
			collisionFrac.x = 1.0f;
			collisionFrac.y = 1.0f;
			state = 0;
			timer = _wait;
			E_ASSERT(timer > 0);
			this->isEllipseCollision = false;
		}

		bool Step() override
		{
			ani.Step();
			tex = ani.GetTex();
			switch(state)
			{
			case 0:
				E_ASSERT(timer < 100000);
				if(--timer == 0)
				{
					state = 1;
					timer = 120;
					acc = -130 * 2 /float(timer)/float(timer);
					vel = -acc * timer;
					kxxwin->EarthQuake(0.1f);
					kxxwin->PlaySE("enemy-shot-0", pos, 0.5f);
				}
				break;
			case 1:
				vel+= acc;
				pos.y+=vel;
				if(--timer == 0)
				{
					state = 2;
					timer = 80;
				}
				break;
			case 2:
				if(--timer == 0)
				{
					state = 1;
					acc = 0.013f + 0.003f * kxxwin->state.level;
					vel = 0;
				}
				break;
			case 3:
				vel+= acc;
				pos.y+=vel;
				break;
			}
			return pos.y - hsz.y * scl.y < K_GAME_H + 10;
		}
	};

	BossBNA1::BossBNA1()
	{
	}

	void BossBNA1::Begin()
	{
		time_limit = 8 * (pb()->ani_stand_right.GetTotalSpan() + pb()->ani_fire.GetTotalSpan());
		InitState0();
	}

	void BossBNA1::InitState0()
	{
		state = 0;
		ani = pb()->ani_fire;
		ani.Reset();
		timer = ani.GetTotalSpan();
		boss->tex = ani.GetTex();
		int n = 8 + 2 * kxxwin->state.level;
		float x = kxxwin->player->pos.x - 10 * n;
		uint32 wait = 1;
		for(int i=0; i<n; i++)
		{
			BossBNA1Shot * p = enew BossBNA1Shot(wait);
			p->pos.x = x;
			p->pos.y = -70.0f;
			kxxwin->AddEnemyShotToList(p);
			wait+=30;
			x+= 20;
		}
	}

	void BossBNA1::InitState1()
	{
		state = 1;
		ani = pb()->ani_stand_left;
		ani.Reset();
		timer = ani.GetTotalSpan();
		boss->tex = ani.GetTex();
	}

	bool BossBNA1::Step()
	{
		if(--timer == 0)
		{
			if(state == 0 )
			{
				InitState1();
			}
			else
			{
				InitState0();
			}
		}
		else
		{
			ani.Step();
			boss->tex = ani.GetTex();
		}
		
		return true;
	}

}
