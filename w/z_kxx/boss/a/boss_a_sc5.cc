#include <z_kxx/boss/a/boss_a_sc5.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
	class BossASC5Petal : public EnemyShot
	{
		int state;
		uint32 timer;
		uint32 timer_state1;
		Vector2 center;
		float angle;
		float angle_delta;
		float radius1;
		float scale_delta;
	public:
		BossASC5Petal(const Vector2 & _pos, float _w, float _h, float _r0, float _a0, float _da, uint32 _t, uint32 _delay)
		{
			center = _pos;
			angle = _a0;
			angle_delta = _da;
			hsz.x = _w * 0.5f;
			hsz.y = _h * 0.5f;
			radius1 = _r0 + hsz.y;
			timer_state1 = _t;
			state = 0;
			timer = _delay + 1;
			scale_delta = 1.0f / timer;
			scl.x = scl.y = 0.0f;
			Step();
		}

		bool Step() override
		{
			if(--timer == 0)
			{
				switch(state)
				{
				case 0:
					timer = 120;
					state = 1;
					scale_delta = 1.0f / timer;
					break;
				case 1:
					timer = timer_state1;
					state = 2;
					scale_delta = 0;
					scl.x = scl.y = 1.0f;
					break;
				case 2:
					timer = 120;
					state = 3;
					scale_delta = -0.99f / timer;
					break;
				case 3:
					return false;
				}
			}

			if(state)
			{
				angle+= angle_delta;
				ang = angle+ 0.5f * PI;
				scl.x = scl.y = scl.y + scale_delta;
				pos.x = center.x + (cos(angle) * radius1) * scl.y;
				pos.y = center.y + (sin(angle) * radius1) * scl.y;
			}
			return true;
		}

		bool Collide(const Vector2 & _v, float _r) override
		{
			return state ? EnemyShot::Collide(_v, _r) : false;
		}

	};

	static void AddSC5FlowerShot(const Vector2 & _pos)
	{
		{
			TexRef tex1 = kxxwin->shotTex[3][INDEX_F];
			for(float a = 0; a < 2*PI; a+= 0.125f*PI)
			{
				BossASC5Petal * p = enew BossASC5Petal(_pos, 32, 96, 20, a, -0.01f, 480, 180);
				p->tex = tex1;
				kxxwin->AddEnemyShotToList(p, false);
			}
		}

		{
			TexRef tex1 = kxxwin->shotTex[13][0];
			for(float a = 0; a < 2*PI; a+= 0.125f*PI)
			{
				BossASC5Petal * p = enew BossASC5Petal(_pos, 32, 80, 10, a, 0.01f, 480, 120);
				p->tex = tex1;
				kxxwin->AddEnemyShotToList(p, false);
			}
		}	
		{
			TexRef tex1 = kxxwin->shotTex[3][INDEX_D];
			for(float a = 0; a < 2*PI; a+= 0.125f*PI)
			{
				BossASC5Petal * p = enew BossASC5Petal(_pos, 20, 20, 12, a, -0.01f, 480, 60);
				p->tex = tex1;
				kxxwin->AddEnemyShotToList(p, false);
			}
		}
		{
			TexRef tex1 = kxxwin->shotTex[3][INDEX_J];
			for(float a = 0; a < 2*PI; a+= 0.25f*PI)
			{
				BossASC5Petal * p = enew BossASC5Petal(_pos, 10, 32, -16, a, 0.01f, 480, 0);
				p->tex = tex1;
				kxxwin->AddEnemyShotToList(p, false);
			}
		}
	}

	class BossASC5SmallPetal : public EnemyShot
	{
		uint32 timer;
		Vector2 speed;
		Vector2 accel;
	public:

		BossASC5SmallPetal(float _x, float _y, float _a)
		{
			tex = kxxwin->shotTex[0][0];
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;

			timer = (int)(1.0f * K_LOGIC_FPS);
			float speed0_f = shot_vel(10);
			float speed1_f = shot_vel(5);
			float sin_a = sin(_a);
			float cos_a = cos(_a);
			float accel_f = (speed1_f - speed0_f) / timer;
			accel.x = accel_f * cos_a;
			accel.y = accel_f * sin_a;
			speed.x = speed0_f * cos_a;
			speed.y = speed0_f * sin_a;
			pos.x = _x + speed.x;
			pos.y = _y + speed.y;
			RotateNorthToVectorDirection(speed);
			hue = 3;
		}

		bool Step()
		{
			if(--timer)
			{
				pos.x+= speed.x;
				pos.y+= speed.y;
				speed.x+= accel.x;
				speed.y+= accel.y;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool Collide(const Vector2 & _v, float _r)
		{
			float a = -PI * 0.5f - ang;
			float b = hsz.y * 0.5f;
			float x0 = b * cos(a);
			float y0 = b * sin(a);
			float dx = _v.x - x0 - pos.x;
			float dy = _v.y - y0 - pos.y;
			_r = _r + 3;
			return dx*dx + dy*dy <= _r * _r;
		}

		void RenderDebug()
		{
#ifdef NB_DEBUG
			RGBA color = kxxwin->debugFontColor;
			float a = -PI * 0.5f + ang;
			float b = hsz.y * 0.5f;
			float x0 = b * cos(a);
			float y0 = b * sin(a);
			graphics->DrawEllipse(pos.x+x0, pos.y+y0, ang, 3, 3, color, false);
#endif
		}
	};

	class BossASC5SmallFlower : public EnemyShot
	{
		uint32 timer;
		float scale_delta;
	public:
		BossASC5SmallFlower(TexRef & _tex)
		{
			tex = _tex;
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;
			timer = 120;
			scl.x = scl.y = 0;
			scale_delta = 1.0f / timer;
		}
		bool Step() override
		{
			scl.x = scl.y = scl.x + scale_delta;
			if(--timer == 0)
			{
				float a0 = logic_random_float() * PI;
				for(float a = 0; a < 2.0f * PI; a+= 0.5f * PI)
				{
					BossASC5SmallPetal * p = enew BossASC5SmallPetal(pos.x, pos.y, a + a0);
					kxxwin->AddEnemyShotToList(p, false);
				}
				return false;
			}
			else
			{
				return true;
			}
		}

	};


	class BossASC5MasterShot : public EnemyShot
	{
		int state;
		uint32 timer;
	public:
		uint32 flower_timer;
		Vector2 vel;
		BossASC5MasterShot()
		{
		}

		void Init(uint32 _delay)
		{
			if(_delay)
			{
				state = 0;
				timer = _delay;
			}
			else
			{
				state = 1;
				timer = 1;
			}
		}

		bool Step() override
		{
			if(state)
			{
				pos+=vel;
				if(--timer == 0)
				{
					timer = 30;
					Vector2 v;
					v.x = pos.x + (logic_random_float() - 0.5f) * 32.0f;
					v.y = pos.y + (logic_random_float() - 0.5f) * 32.0f;
					BossASC5SmallFlower * p = enew BossASC5SmallFlower(kxxwin->shotTex[6][0]);
					p->pos = v;
					kxxwin->AddEnemyShotToList(p, false);
				}
				if(--flower_timer ==0)
				{
					flower_timer = 9999;
					AddSC5FlowerShot(pos);
				}
			}
			else
			{
				if(--timer == 0)
				{
					state = 1;
					timer = 1;
				}
			}
			return !DisappearTest();
		}

		void Render() override
		{
		}

		bool Collide(const Vector2 & _v, float _r) override
		{
			return false;
		}
	};


	BossASC5::BossASC5()
	{
		sc_name = _TT("boss-a-sc5-name");
	}

	void BossASC5::Begin()
	{
		this->time_limit = 30 * K_LOGIC_FPS;

		shot_timer = S2F(1.25f);

		kxxwin->PlaySE("spell-card", boss->pos);
		if(boss->pets)
		{
			BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
			if(p)
			{
				p->state = ES_LING_SNIPE;
				p->ethereal = false;
				p->state = -1;
			}
		}
		boss->ShowSCName(sc_name);
		boss->ShowAvatar();
		boss->ShowRing(true);
		kxxwin->AddBlastWave(boss->pos, false);
	}

	bool BossASC5::Step()
	{
		if(--shot_timer==0)
		{
			shot_timer = S2F(2.00f);
			//AddSC5FlowerShot(kxxwin->player->pos);
			Vector2 v = kxxwin->player->pos;
			if(v.y > K_GAME_H)
			{
				v.y = K_GAME_H;
			}
			BossASC5MasterShot * px = enew BossASC5MasterShot();
			BossASC5MasterShot * py = enew BossASC5MasterShot();
			px->vel.y = 0;
			px->pos.y = v.y;
			py->vel.x = 0;
			py->pos.x = v.x;
			float vel = shot_vel(60);
			if(v.x < K_GAME_XC)
			{
				px->pos.x = K_GAME_W;
				px->vel.x = -vel;
			}
			else
			{
				px->pos.x = 0;
				px->vel.x = vel;
			}

			if(v.y < K_GAME_YC)
			{
				py->pos.y = K_GAME_H;
				py->vel.y = -vel;
			}
			else
			{
				py->pos.y = 0;
				py->vel.y = vel;
			}
			float dx = fabs(px->pos.x - v.x);
			float dy = fabs(py->pos.y - v.y);
			if(dx < dy)
			{
				px->flower_timer = 99999;
				py->flower_timer = uint32(dy / vel);
				py->Init(0);
				px->Init(uint32((dy-dx) / vel));
			}
			else
			{
				px->flower_timer = uint32(dx / vel);
				py->flower_timer = 99999;
				px->Init(0);
				py->Init(uint32((dx-dy) / vel));
			}
			kxxwin->AddEnemyShotToList(px);
			kxxwin->AddEnemyShotToList(py);
		}
		return true;
	}

	void BossASC5::End()
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
