
#include <z_kxx/stage/1/stage_1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player.h>
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/stage/kxx_dialog.h>
#include <z_kxx/boss/a/boss_a.h>
#include <z_kxx/boss/a/boss_a_mid.h>
#include <z_kxx/util/util_func.h>

#include <z_kxx/stage/simple_stage_bg.h>
#include <z_kxx/stage/test_stage_bg.h>

namespace e
{
	Stage1::Stage1()
		: Stage(1-1)
	{
		SimpleStageBG * p = enew SimpleStageBG(human_readable_id());
		p->fog_color.r = 0.2f;
		p->fog_color.g = 0.25f;
		p->fog_color.b = 0.2f;
		p->fog_color.a = 0.2f;
		bg[0] = p;
		bg[1] = enew TestStageBG();
	}

	Stage1::~Stage1()
	{

	}


	void Stage1::OnCreateBoss(int _index)
	{
		switch(_index)
		{
		case 0:
			enew BossAMid(this, _index);
			break;
		case 1:
			enew BossA(this, _index);
			break;
		default:
			E_ASSERT(0);
			break;
		}
	}

	void Stage1::OnBossFight(int _index)
	{
		E_ASSERT(!dialog);
		if(_index == 1)
		{
			kxxwin->PlayBGM(12);
		}
	}
}

