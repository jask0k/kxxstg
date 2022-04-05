#include <z_kxx/boss/a/boss_a_sc2.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/shot/shot.h>

namespace e
{
	class BossASC2Shot : public EnemyShot
	{
	public:
		Vector2 accel;
		Vector2 vel;
		bool drop;
		void Drop()
		{
			drop = true;
			float a = 0.5f * PI - 0.1f + 0.1f * logic_random_float();
			float accel_f = (0.0003f + 0.0006f * logic_random_float());
			accel.x = cos(a) * accel_f;
			accel.y = sin(a) * accel_f;
			vel.x = 0;
			vel.y = 0;
		}

		bool Step() override
		{
			if(drop)
			{
				vel+= accel;
				pos+= vel;
			}
			else
			{
				return true;
			}
			return pos.y - hsz.y < K_GAME_H;
		}
	};

	static void AddBossASC2Shot(const Vector2 & _pos, Sprite * _master)
	{
		BossASC2Shot * p = enew BossASC2Shot();
		p->pos  = _pos;
		p->drop = false;
		p->SetSize(40);
		p->tex = kxxwin->shotTex[16][0];
		_master->AddPet(p);
		kxxwin->AddFlashSpark(p->pos.x, p->pos.y, p->hue);
		kxxwin->AddEnemyShotToList(p);

	}

	class BossASC2MasterShot : public EnemyShot
	{
	public:
		uint32 shot_timer;
		Vector2 vel;
		BossASC2MasterShot()
		{
			shot_timer = (15 - kxxwin->state.level*2) * K_LOGIC_FPS_MUL / 2;
		}
		bool Step() override
		{
			pos+= vel;
			if(master)
			{
				if(--shot_timer == 0)
				{
					shot_timer = (15 - kxxwin->state.level*2) * K_LOGIC_FPS_MUL;
					AddBossASC2Shot(pos, master);
				}
			}
			return !IsOutOfGameArea(pos, 10);
		}
	};

	BossASC2::BossASC2()
	{
		//sc_name = _TT("Imagine Breaker");
	}

	void BossASC2::Begin()
	{
		//vel.x = 0;
		//vel.y = 0;
		this->init_life = 400;
		this->time_limit = 59 * K_LOGIC_FPS;
		state = 0;
		shot_timer = 1;
		start_pos = boss->pos;
		kxxwin->PlaySE("spell-card", boss->pos);
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = -1;
				p->ethereal = false;
			}
		}
		CalcState0Vel();
		boss->ShowSCName(sc_name);
		boss->ShowAvatar();
		boss->ShowRing(true);
		kxxwin->AddBlastWave(boss->pos, false);
	}

	bool BossASC2::Step()
	{
		switch(state)
		{
		case 0:
			// shot
			boss->pos+= vel;
			if(--shot_timer==0)
			{
				shot_timer = S2F(1.0f);

				if(boss->pos.y < K_GAME_H)
				{
					float angs[4] = {0.25f, 0.75f, 1.25f, 1.75f};
					for(int i=0; i<4; i++)
					{
						BossASC2MasterShot * p;
						p = enew BossASC2MasterShot();
						p->pos = boss->pos;
						p->ang = angs[i] * PI;
						p->vel.x = 2.5f * cos(p->ang);
						p->vel.y = 2.5f * sin(p->ang);
						p->ang+= 0.5f * PI;
						p->SetSize(20);
						p->tex = kxxwin->shotTex[4][0];
						boss->AddPet(p);
						kxxwin->AddEnemyShotToList(p);
					}
				//	AddBossASC2Shot(boss->pos, boss);
					kxxwin->PlaySE("enemy-shot-2", boss->pos, 0.8f);
				}
			}
			if(boss->pos.y > K_GAME_H + 130)
			{

				Vector2 v;
				v.y = start_pos.y;
				v.x = 100 + (K_GAME_W - 200) * logic_random_float();
				v = v - boss->pos;
				vel = v.GetNormal() * 1.2f;
		
				state = 1;

				Sprite * p = boss->pets;
				while(p)
				{
					BossASC2Shot * p1 = dynamic_cast<BossASC2Shot*>(p);
					if(p1)
					{
						p1->Drop();
					}
					p = p->pet_next;
				}
				shot_timer = 1;
			}
			break;
		case 1:
			// go back
			boss->pos+= vel;
			if(boss->pos.y <= start_pos.y)
			{
				state = 2;
				state2_timer = S2F(2.0f);
				CastPetShot();
			}
			break;
		case 2:
			// rest
			if(--state2_timer == 0)
			{
				state = 0;
				CalcState0Vel();
				CastPetShot();
			}
			break;
		default:
			E_ASSERT(0);
			break;
		}
		return true;
	}

	void BossASC2::CastPetShot()
	{
		Sprite * p = boss->pets;
		while(p)
		{
			BossAPet * p1 = dynamic_cast<BossAPet*>(p);
			if(p1)
			{
				AddEnemyShot(p1->pos, ES_LING_SNIPE);
			}
			p = p->pet_next;
		}
	}

	void BossASC2::CalcState0Vel()
	{
		Vector2 v;
		v.y = K_GAME_H + 130;
		v.x = 50 + (K_GAME_W - 100) * logic_random_float();
		v = v - boss->pos;
		vel = v.GetNormal() * 0.5f;
	}

	void BossASC2::End()
	{
		SCScript::End();
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = -1;
				p->ethereal = true;
			}
		}
		boss->ShowRing(false);
		kxxwin->ClearEnemyShots(false, 10);
		boss->SetDark(100.0f, 0.1f);
		boss->HideSCName();
	}
}
