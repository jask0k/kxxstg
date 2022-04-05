
// #include "../config.h"
#include <z_kxx/stage/3/stage_3.h>
#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
//#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/boss/a/boss_a.h>

namespace e
{
	Stage3::Stage3()
		: Stage(3-1)
	{
		SimpleStageBG * p = enew SimpleStageBG(human_readable_id());
		p->fog_color.r = 0.25f;
		p->fog_color.g = 0.2f;
		p->fog_color.b = 0.0f;
		p->fog_color.a = 0.2f;
		bg[0] = p;
	}

	Stage3::~Stage3()
	{
	}

	void Stage3::OnCreateBoss(int _index)
	{
		//E_ASSERT(_index == 0);
		// enew BossB(this, _index);
	}

	void Stage3::OnBossFight(int _index)
	{
		//E_ASSERT(_index == 0);
		//kxxwin->PlayBGM(22);
	}
}

