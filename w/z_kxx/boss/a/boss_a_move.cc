#include <z_kxx/boss/a/boss_a_move.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	BossAMove::BossAMove(uint32 _time, float _to_x, float _to_y)
	{
		timer = _time;
		to.x = _to_x;
		to.y = _to_y;
		stand = false;
	}

	void BossAMove::Begin()
	{
//		boss->ethereal = true;
		Vector2 v = to - boss->pos;
		if(v.x > 0)
		{
			ani = boss->ani_move_right;
		}
		else
		{
			ani = boss->ani_move_left;
		}
		ani.Reset();
		uint32 t1 = ani.GetTotalSpan();
		if(t1 < 0)
		{
			t1 = 1;
		}
		if(timer < t1)
		{
			timer = t1;
		}
		time_limit = timer;

		float s = v.length();
		vel_f = s / t1;
		vel = v.GetNormal() * vel_f;
	}

	bool BossAMove::Step()
	{
		if(stand)
		{
			if(ani.Step())
			{
				boss->tex = ani.GetTex();
			}
		}
		else
		{
			if(ani.Step())
			{
				boss->tex = ani.GetTex();
				boss->pos+= vel;
			}
			else 
			{
				stand = true;
				if(vel.x > 0)
				{
					ani = boss->ani_stand_right;
				}
				else
				{
					ani = boss->ani_stand_left;
				}
				ani.Reset();
				boss->tex = ani.GetTex();
			}
		}
		return true;
	}

}
