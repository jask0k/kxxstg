#include <z_kxx/boss/b/boss_b_sc1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/laser.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
	class BossBSC1Laser : public Laser
	{
		uint32 drop_timer;
	public:
		BossBSC1Laser(Sprite * _master, TexRef _tex, float _w, float _a, float _da, float _r0, float _r, uint32 _t)
			: Laser(_master, _tex, _w, _a, _da, _r0, _r, _t)
		{
			drop_timer = 80;
		}

		bool Step() override
		{
			if(--drop_timer == 0)
			{
				drop_timer = 80;
				if(master && master->pos.y > 0)
				{
					float a1 = angle_normalize(ang);
					if(a1 < 0)
					{
						float dy = 0 - master->pos.y;
						float dx = dy / tan(a1);
						Vector2 v = {master->pos.x + dx, master->pos.y + dy};
						AddEnemyShot(v, ES_SHOTGUN);
					}
				}
			}
			return Laser::Step();
		}
	};

	BossBSC1::BossBSC1()
	{
		time_limit = S2F(40);
		sc_name = L"boss-b-sc1-name";
	}

	void BossBSC1::Begin()
	{
		kxxwin->PlaySE("spell-card", boss->pos);
		//if(boss->pets)
		//{
		//	BossBPet * p = dynamic_cast<BossBPet*>(boss->pets);
		//	if(p)
		//	{
		//		//p->state = ES_LING_SCATTER;
		//		p->state = -1;
		//		p->ethereal = false;
		//	}
		//}
		boss->ShowSCName(sc_name);
		boss->ShowAvatar();
		boss->ShowRing(true);
		kxxwin->AddBlastWave(boss->pos, false);

		Vector2 c;
		c.x = K_GAME_XC;
		c.y = 180;
		float r = 130;
		float a = -0.5f * PI;
		float da = 2*PI/5;
		for(int i=0; i<5; i++, a+=da)
		{
			star[i].x = c.x + cos(a) * r;
			star[i].y = c.y + sin(a) * r;
		}
		star_index = 0;
		CalcState0();
	}

	void BossBSC1::CalcState0()
	{
		state = 0;
		Vector2 v = (star[star_index] - boss->pos);
		float dis = v.length();
		float vel_f = PS2PF(170);
		timer = (int)(dis / vel_f);
		if(timer == 0)
		{
			timer = 1;
		}
		vel = v.GetNormal() * vel_f;

		throw_timer = 1;
		throw_a = star_index * 0.4f*PI + PI * 0.65f;
		throw_da = 0.045f * PI;

	}

	bool BossBSC1::Step()
	{
		switch(state)
		{
		case 0:
			if(--throw_timer==0)
			{
				throw_timer = S2F(0.15f);
				float speed_x = shot_vel(45) * cos(throw_a);
				float speed_y = shot_vel(45) * sin(throw_a);
				/*
				BossBThrowShot * p = enew BossBThrowShot(1, speed_x, speed_y, 0);
				p->pos = boss->pos;
				p->SetSize(40);
				kxxwin->AddEnemyShotToList(p);
				kxxwin->PlaySE("enemy-shot-2", boss->pos, 0.5f);
				throw_a+= throw_da;
				*/
				float a = -0.5f * PI;
				float da = 0.001f * PI;
				Laser * p = enew BossBSC1Laser(boss, kxxwin->shotTex[15][INDEX_F], 8, a, da, 20, 560, 480);
				kxxwin->AddEnemyShotToList(p);

			}
			if(--timer == 0)
			{
				state = 1;
				timer = S2F(3.0f);
			}
			else
			{
				boss->pos+=vel;
			}
			break;
		case 1:
			if(--timer == 0)
			{
				star_index = star_index - 2;
				if(star_index < 0)
				{
					star_index+= 5;
				}
				CalcState0();
			}
			break;
		}
		return true;
	}

	void BossBSC1::End()
	{
		SCScript::End();
		//if(boss->pets)
		//{
		//	BossBPet * p = dynamic_cast<BossBPet*>(boss->pets);
		//	if(p)
		//	{
		//		p->state = -1;
		//		p->ethereal = true;
		//	}
		//}
		boss->ShowRing(false);
		kxxwin->ClearEnemyShots(false, 10);
		boss->SetDark(100.0f, 0.1f);
		boss->HideSCName();
	}
}
