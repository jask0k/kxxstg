
// #include "../config.h"
#include <z_kxx/stage/4/stage_4.h>
#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
//#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/boss/a/boss_a.h>

namespace e
{
	Stage4::Stage4()
		: Stage(4-1)
	{
		SimpleStageBG * p = enew SimpleStageBG(human_readable_id());
		p->fog_color.r = 0.3f;
		p->fog_color.g = 0.3f;
		p->fog_color.b = 0.3f;
		p->fog_color.a = 0.1f;
		bg[0] = p;
	}

	Stage4::~Stage4()
	{
	}

	void Stage4::OnCreateBoss(int _index)
	{
		//E_ASSERT(_index == 0);
		// enew BossB(this, _index);
	}

	void Stage4::OnBossFight(int _index)
	{
		//E_ASSERT(_index == 0);
		//kxxwin->PlayBGM(22);
	}
}

