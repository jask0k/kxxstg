
// #include "../config.h"
#include <z_kxx/stage/2/stage_2.h>
#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player.h>
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/stage/kxx_dialog.h>
#include <z_kxx/boss/b/boss_b.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	Stage2::Stage2()
		: Stage(2-1)
	{
		SimpleStageBG * p = enew SimpleStageBG(human_readable_id());
		p->fog_color.r = 0.2f;
		p->fog_color.g = 0.6f;
		p->fog_color.b = 0.7;
		p->fog_color.a = 0.2f;
		bg[0] = p;
	}

	Stage2::~Stage2()
	{
	}

	void Stage2::OnCreateBoss(int _index)
	{
		E_ASSERT(_index == 0);
		enew BossB(this, _index);
	}

	void Stage2::OnBossFight(int _index)
	{
		E_ASSERT(_index == 0);
		E_ASSERT(!dialog);
		kxxwin->PlayBGM(22);
	}
}

