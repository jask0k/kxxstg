
// #include "../config.h"
#include <z_kxx/enemy/wild_flower.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/shot/laser.h>

namespace e
{
#define STOP_Y 200
#define STOP_TIME (240*3)
#define MOVE_TIME S2F(2.0f)

	class WildFlowerShot : public EnemyShot
	{
	public:
		Ani ani;
		Vector2 vel;
		Vector2 acc;
		float a;
		uint32 wait;
		uint32 timer;
		int state;
		WildFlowerShot(TexRef & _tex, const Vector2 & _v, float _a, uint32 _wait)
		{
			//tex = _tex;
			ani = kxxwin->LoadAni("test-ani-shot");
			tex = ani.GetTex();
			wait = _wait;
			pos = _v;
			a = _a;
			ang = a - PI * 0.5f;
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;
			state = 0;
			timer = 60;
			float vel_f = shot_vel(70);
			vel.SetPolar(vel_f, ang);
			float acc_f = -vel_f / timer;
			acc.SetPolar(acc_f, ang);
			isEllipseCollision = false;
		}

		~WildFlowerShot()
		{
		}

		bool Step() override
		{
			switch(state)
			{
			case 0:
				if(--timer == 0)
				{
					float vel_f = shot_vel(25) *  (1.0f + 0.2f * sin(a * 5.0f + PI));
					vel.SetPolar(vel_f, a);
					state = 1;
					timer = wait;
				}
				else
				{
					vel+= acc;
					pos+= vel;
				}
				return true;
			case 1:
				if(--timer == 0)
				{
					state = 2;
					if(master)
					{
						master->RemovePet(this);
					}
				}
				return true;
			case 2:
				pos+= vel;
				if(ani.Step())
				{
					tex = ani.GetTex();
				}
				break;
			}
			return !DisappearTest();
		}
	};

	WildFlower::WildFlower(int _pos, int _drop)
	{
		init_life = life = 30;
		def = 0.2f;
		dr = 0.2f;
		pos.y = -20.0f;
		float pos0 = 120.0f;
		switch(_pos)
		{
		case 0:
			pos.x = K_GAME_XC - pos0;
			break;
		case 1:
			pos.x = K_GAME_XC + pos0;
			break;
		default:
			if(logic_random_int() & 0x01)
			{
				pos.x = K_GAME_XC - pos0;
			}
			else
			{
				pos.x = K_GAME_XC + pos0;
			}
			break;
		}

		SetSize(40);

		tex = kxxwin->LoadTex("flower");
		drops.Set(0, _drop, 1, 1);

		state = 0;
		timer = MOVE_TIME;
		float a = 2.0f * float(STOP_Y) / (float(timer) * float(timer));
		acc.y = -a;
		acc.x = 0;
		vel.x = 0;
		vel.y = a * timer;
		shot_skip = 4 - kxxwin->state.level;
	}


	WildFlower::~WildFlower()
	{
	}

	bool WildFlower::Step()
	{
		Enemy::Step();

		ang+= 0.05f;

		switch(state)
		{
		case 0:
			if(--timer==0)
			{
				state = 1;
				timer = STOP_TIME;
			}
			else
			{
				vel+= acc;
				pos+= vel;
			}
			return true;
		case 1:
			if(--timer==0)
			{
				state = 2;
				vel.x = 0;
				vel.y = 0;
			}
			else
			{
				int t = (STOP_TIME - timer-1);
				int w = 60;
				if(t < (120 + w)*3  && t % shot_skip == 0)
				{
					int t1 = t % (120 + w);
					if(t1 < 120)
					{
						int h = t / (120 + w);
						float r0 = 20.0f;
						float a = (t1 / 120.0f) * 2 * PI;
						Vector2 v = pos;
						v.x+= r0 * cos(a);
						v.y+= r0 * sin(a);
						EnemyShot * p = enew WildFlowerShot(kxxwin->shotTex[1][h], v, a, 120 + 60 - t1);
						p->hue = h;
						this->AddPet(p);
						kxxwin->AddEnemyShotToList(p);
						//AddEnemyShot(pos, ES_LASER_FAN);
					}
				}
			}
			return true;
		case 2:
			vel+= acc;
			pos+= vel;
			return !IsOutOfGameArea(pos, 42);
		}
		return false;
	}
}

