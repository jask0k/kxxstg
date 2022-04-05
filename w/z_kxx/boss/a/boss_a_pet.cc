#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	BossAPet::BossAPet(const Vector2 & _pos)
	{
		pos = _pos;
		delay_master_pos = pos;
		target_a = 0;
		target_b = 0;
		life = init_life = 500;
		velocity.x = 0;
		velocity.y = 0;
		tex = kxxwin->LoadTex("flower");
		SetSize(40);
		def = 0.3f;
		dr  = 0.2f;
		shot_timer = S2F(1.5f);
		state = -1;
	}

	bool BossAPet::Step()
	{
		if(master)
		{
			Vector2 t;
			t.x = cos(target_a) * 61;
			t.y = sin(target_a) * 61;
			t.x+= cos(target_b) * 50;
			t.y+= sin(target_b) * 50;
			PositionApproach(delay_master_pos, master->pos, 0.25f);
			/*Vector2 f = master->pos + t - pos;
			f.x = f.x * 0.02f;
			f.y = f.y * 0.02f;
			velocity+= f / 3.0f;
			float velf = velocity.LengthSquared();
			if(velf > 25)
			{
				velocity = velocity.GetNormal() * 2;
			}*/
			pos = delay_master_pos + t;
		}

		target_a+= 0.003f;
		target_b+= 0.002f;

		ang-= 0.04f;
		if(state >= 0)
		{
			if(--shot_timer == 0)
			{
				shot_timer = S2F(1.5f - kxxwin->state.level * 0.2f);
				AddEnemyShot(pos, state);
			}
		}
		return true;
	}
}
