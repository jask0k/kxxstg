
// #include "../config.h"
#include <z_kxx/stage/5/stage_5.h>
#include <z_kxx/stage/5/gallery.h>
#include <z_kxx/main/kxxwin.h>
//#include <z_kxx/player/player.h>
//#include <z_kxx/enemy/enemy.h>
//#include <z_kxx/boss/a/boss_a.h>

namespace e
{
	Stage5::Stage5()
		: Stage(5-1)
	{
		Gallery * p = enew Gallery();
		bg[0] = p;
	}

	Stage5::~Stage5()
	{
	}

	void Stage5::OnCreateBoss(int _index)
	{
		//E_ASSERT(_index == 0);
		// enew BossB(this, _index);
	}

	void Stage5::OnBossFight(int _index)
	{
		//E_ASSERT(_index == 0);
		//kxxwin->PlayBGM(22);
	}
}

