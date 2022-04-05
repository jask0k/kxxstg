#include <z_kxx/boss/a/boss_a_na1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_pet.h>

namespace e
{
	//  0 ---1--->
	//   <---3--- 2
	BossANA1::BossANA1()
	{
		state = 0;
		shot_timer = S2F(0.3f);
		shot_counter = 10;
	}

	void BossANA1::Begin()
	{
		BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
		if(p)
		{
			p->state = ES_LING_SNIPE;
		}
	}

	bool BossANA1::Step()
	{
		if(state == 0 || state == 2)
		{
			if(--shot_timer==0)
			{
				shot_timer = S2F(0.4f - 0.1f * kxxwin->state.level);
				AddEnemyShot(boss->pos, ES_LING_SCATTER);
				if(--shot_counter==0)
				{
					state++;
					shot_counter = 10;
				}
			}
		}
		else if(state == 1)
		{
			if(boss->pos.x < K_GAME_W - 160)
			{
				boss->pos.x+= PS2PF(300);
			}
			else
			{
				state = 2;
			}
		}
		else if(state == 3)
		{
			if(boss->pos.x > 160)
			{
				boss->pos.x-= PS2PF(300);
			}
			else
			{
				state = 0;
			}
		}
		else
		{
			E_ASSERT(0);
		}
		return true;
	}

	void BossANA1::End()
	{
		NAScript::End();
		BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
		if(p)
		{
			p->state = -1;
		}
		kxxwin->ClearEnemyShots(false, 10);
	}
}
