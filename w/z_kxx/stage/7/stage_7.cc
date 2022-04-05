
// #include "../config.h"
#include <z_kxx/stage/7/stage_7.h>
#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
//#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/boss/a/boss_a.h>

namespace e
{
	Stage7::Stage7()
		: Stage(7-1)
	{
		SimpleStageBG * p = enew SimpleStageBG(human_readable_id());
		p->fog_color.r = 0.0f;
		p->fog_color.g = 0.0f;
		p->fog_color.b = 0.0f;
		p->fog_color.a = 0.3f;
		bg[0] = p;
	}

	Stage7::~Stage7()
	{
	}

	void Stage7::OnCreateBoss(int _index)
	{
		//E_ASSERT(_index == 0);
		// enew BossB(this, _index);
	}

	void Stage7::OnBossFight(int _index)
	{
		//E_ASSERT(_index == 0);
		//kxxwin->PlayBGM(22);
	}
}

