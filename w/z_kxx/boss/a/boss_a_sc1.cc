#include <z_kxx/boss/a/boss_a_sc1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/boss/a/boss_a_sc1_bg.h>

namespace e
{

	class ThrowShot : public EnemyShot
	{
		Vector2 speed;
		float accel;
		int type;
	public:
		ThrowShot(int _type, float _speed_x, float _speed_y, float _accel);
		bool Step() override;
		void DropItem() override
		{
			if(type == 1)
			{
				for(float a = 0; a < 2*PI; a+= 1.257f)
				{
					kxxwin->AddDrop(DROP_TINY_POINT, pos.x + cos(a) * this->hsz.x, pos.y + sin(a) * this->hsz.y, false, -(PI * 0.5f));
				}
			}
			else
			{
				kxxwin->AddDrop(DROP_TINY_POINT, pos.x , pos.y, false, -(PI * 0.5f));
			}
		}
	};

	ThrowShot::ThrowShot(int _type, float _speed_x, float _speed_y, float _accel)
	{
		type = _type;
		switch(type)
		{
		case 1:
			tex = kxxwin->shotTex[6][0];
			break;
		}
		accel = _accel;
		speed.x = _speed_x;
		speed.y = _speed_y;
	}

	bool ThrowShot::Step()
	{
		float r = hsz.y * 0.5f;
		if(pos.y > r && pos.y < K_GAME_H - r && pos.x > r && pos.x < K_GAME_W - r)
		{
			speed.y+= accel;
			pos.x+= speed.x;
			pos.y+= speed.y;
			return true;
		}
		else
		{
			switch(type)
			{
			case 1:
				{
					float a0 = LogicRandomAngle();
					int n = 10 + kxxwin->state.level * 5;
					float da = PI * 2.0f / n;
					for(float a = 0; a < PI*2; a+= da)
					{
						float v = shot_vel(30);
						float vx = cos(a+a0) * v;
						float vy = sin(a+a0) * v;
						// a = v^2/(2*s)
						float acc = (v * v) * 0.5f / 500.0f;

						ThrowShot * p = enew ThrowShot(0, vx, vy, acc);
						p->tex = kxxwin->shotTex[7][0];
						p->pos.x = pos.x + vx;
						p->pos.y = pos.y + vy;
						p->SetSize(12);
						kxxwin->AddEnemyShotToList(p);
					}
					kxxwin->AddExplosion(pos, 80, hue);
					kxxwin->PlayEnemyDeadSE(pos);
				}
				break;
			default:
				kxxwin->AddSmallSpark(pos, 3, hue);
				break;
			}
			return false;
		}
	}

	BossASC1::BossASC1()
	{
		time_limit = S2F(40);
	}

	void BossASC1::Begin()
	{
		boss->stage->bg[2] = enew BossASC1BG();
		kxxwin->PlayBGM(666);
		kxxwin->PlaySE("spell-card", boss->pos);
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				//p->state = ES_LING_SCATTER;
				p->state = -1;
				p->ethereal = false;
			}
		}
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

	void BossASC1::CalcState0()
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

	bool BossASC1::Step()
	{
		switch(state)
		{
		case 0:
			if(--throw_timer==0)
			{
				throw_timer = S2F(0.15f);
				float speed_x = shot_vel(45) * cos(throw_a);
				float speed_y = shot_vel(45) * sin(throw_a);
				ThrowShot * p = enew ThrowShot(1, speed_x, speed_y, 0);
				p->pos = boss->pos;
				p->SetSize(32);
				kxxwin->AddEnemyShotToList(p);
				kxxwin->PlaySE("enemy-shot-2", boss->pos, 0.5f);
				throw_a+= throw_da;
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

	void BossASC1::End()
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
		boss->stage->bg[2]->FadeOut();
	}
}
