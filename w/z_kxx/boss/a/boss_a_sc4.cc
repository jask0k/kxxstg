#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/boss/a/boss_a_sc4.h>
#include <z_kxx/shot/worm_shot.h>

namespace e
{

	class BossASC4WormShot : public EnemyShot
	{
		Worm worm;
		Array<Vector2> curve;
		uint sections;
		uint currentSection;
		Vector2 state2Vel;
		uint32 timer;
	public:
		BossASC4WormShot(Vector2 & _c, uint _sections, float _inner_r, float _outer_r, float _angle)
		{
			E_ASSERT(_sections > 1);
			timer = 1;
			sections = _sections;
			curve.resize(sections);
			const float t0 = -1.207f;
			float cos_a = cos(-_angle);
			float sin_a = sin(-_angle);
			float x0 = t0 * t0;
			float dis = (_outer_r - _inner_r) / x0;
			float dt = (1.414f - t0) / sections;
			int i = 0;
			float t = t0;

			float t1 = t;
			float x = t1*t1 * dis + _inner_r;
			float y = 0.57735026918962f * (t1*t1*t1 - t1) * dis ;
			curve[0].x = x * cos_a - y * sin_a;
			curve[0].y = y * cos_a + x * sin_a;
			i++;
			t+=dt;
			float prev_x = curve[0].x;
			float prev_y = curve[0].y;
			for(; i<sections; i++, t+=dt)
			{
				t1 = t;
				float x = t1*t1 * dis + _inner_r;
				float y = 0.57735026918962f * (t1*t1*t1 - t1) * dis ;
				float x1 = x * cos_a - y * sin_a;
				float y1 = y * cos_a + x * sin_a;
				curve[i].x = x1 - prev_x;
				curve[i].y = y1 - prev_y;
				prev_x = x1;
				prev_y = y1;
			}

			Vector2 dv0 = curve[1] - curve[0];

			curve[0]+= _c;
			pos = curve[0];

			currentSection = 0;
			state2Vel = curve[sections-1];
			worm.Init(64, 10, pos, dv0.Angle());
		}
		~BossASC4WormShot()
		{
//			delete[] curve;
		}
		
		bool Step() override
		{
			if(--timer == 0)
			{
				timer = 4;
				if(++currentSection < sections)
				{
					Vector2 & v = curve[currentSection];
					worm.Step(v.x, v.y, v.Angle()-worm.angle);
					pos = worm.LastPos();
					return true;
				}
				else
				{
					worm.Step(state2Vel.x, state2Vel.y, 0);
					pos = worm.LastPos();
					return !worm.Disappear();
				}
			}
			else
			{
				return true;
			}
		}

		void Render() override
		{
			graphics->SetColor(clr);
			graphics->BindTex(tex);
			worm.Render(graphics);
		}

		void RenderDebug() override
		{
			worm.RenderDebug(graphics);
		}

		void DropItem() override
		{
			worm.DropItem(6, hue);
		}

		void OnCrash(const Vector2 & _pt) override
		{
		}

		bool Collide(const Vector2 & _v, float _r) override
		{
			return worm.Collide(_v, _r * collisionFrac.x);
		}

	};

	BossASC4::BossASC4()
	{
		//sc_name = _TT("Dark Matter");
	}

	void BossASC4::Begin()
	{
		shot_timer = 1;
		shot2_timer = 1 * K_LOGIC_FPS;
		kxxwin->PlaySE("spell-card", boss->pos);
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = ES_LING_SNIPE;
				p->ethereal = false;
			}
		}
		boss->ShowSCName(sc_name);
		boss->ShowAvatar();
		boss->ShowRing(true);
		kxxwin->AddBlastWave(boss->pos, false);
	}

	bool BossASC4::Step()
	{
		if(--shot_timer==0)
		{
			shot_timer = S2F(2.7f);
			Vector2 v;
			v = kxxwin->player->pos;
			if(v.y > K_GAME_H - 30)
			{
				v.y = K_GAME_H - 30;
			}
			float a = 0;
			float da = PI * 0.25f;
			for(int i=0; i<8; i++, a+=da)
			{
				BossASC4WormShot * p = enew BossASC4WormShot(v, 120, 80.0f - kxxwin->state.level * 10.0f, 250, a);
				p->tex = kxxwin->shotTex[16][3+i];
				if(kxxwin->AddEnemyShotToList(p))
				{
					kxxwin->PlaySE("enemy-shot-laser", p->pos, 0.1f);
				}
			}
		}
		if(--shot2_timer==0)
		{
			shot2_timer = S2F(2.0f);
			AddEnemyShot(boss->pos, ES_SHOTGUN);
		}
		return true;
	}

	void BossASC4::End()
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
