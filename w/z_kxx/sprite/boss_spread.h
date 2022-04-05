
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class BossSpread : public Sprite
	{
		uint32 timer;
		uint32 init_timer;
		Vector2 pos;
		TexRef tex;
		//static const int SPREAD_TIME_LIMIT = 30 * K_LOGIC_FPS_MUL;
	public:
		BossSpread(TexRef _tex, const Vector2 & _pos, uint32 _time)
		{
			tex = _tex;
			pos = _pos;
			init_timer = timer = _time;
		}
		bool Step() override
		{
			if(timer == 0)
			{
				return false;
			}
			timer--;
			if(timer %4 == 1)
			{
				int n = 1 + (int)(4 * CalcFadeInFadeOut(init_timer, timer, 0.3f));
				for(int i=0; i<n; i++)
				{
					float a = CrtRandAngle();
					float cos_a = cos(a);
					float sin_a = sin(a);
					float d = 200 + 300 * frand();
					Vector2 v;
					v.x = pos.x;
					v.y = pos.y;

					float speed_f = frand() * 240 + 240;
					float t = d / speed_f;

					float r = (float)tex->W() * 0.5f;
					Spark1 * p = enew Spark1(tex, t, - frand() * PI, frand() * PI, r, r, 0.6f, 0.1f);
					p->pos = v;
					p->speed.x = - PS2PF(speed_f) * cos_a;
					p->speed.y = - PS2PF(speed_f) * sin_a;
					p->blm = BM_ADD;
					p->clr.r = frand() * 0.5f + 0.5f;
					p->clr.g = frand() * 0.5f + 0.5f;
					p->clr.b = frand() * 0.5f + 0.5f;

					kxxwin->AddSparkToList(p, RL_GAME_ENEMY_AURA);
				}
			}
			return true;
		}
	};
}

