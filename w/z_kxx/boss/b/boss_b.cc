
// #include "../config.h"
#include <z_kxx/boss/b/boss_b.h>
#include <z_kxx/boss/b/boss_b_move.h>
#include <z_kxx/boss/b/boss_b_sc1.h>
#include <z_kxx/boss/b/boss_b_na1.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
	BossB::BossB(Stage * _stage, int _index_in_stage)
		: Boss("boss-b", _stage, _index_in_stage)
	{

		ani_stand_left = kxxwin->LoadAni(L"boss-b-ani-stand-left");
		ani_stand_right = kxxwin->LoadAni(L"boss-b-ani-stand-right");
		ani_move_left = kxxwin->LoadAni(L"boss-b-ani-move-left");
		ani_move_right = kxxwin->LoadAni(L"boss-b-ani-move-right");
		ani_fire = kxxwin->LoadAni(L"boss-b-ani-fire");

		tex = ani_stand_left.GetTex();
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;

		aura = enew GeneralAura(kxxwin->enemyAuraTex);
		aura->hsz.x = 128;
		aura->hsz.y = 128;
	//	aura->AddToRenderList(RL_GAME_LOW_AURA);

		aura1 = enew GeneralAura(kxxwin->LoadTex("boss-aura-1"));
		aura1->hsz.x = 100;
		aura1->hsz.y = 100;
//		aura1->AddToRenderList(RL_GAME_LOW_AURA);

		low_ring0 = enew Ring(kxxwin->LoadTex("boss-b-ring"), 120, 128, 0.035f);
		low_ring0->clr.g = 0.7f;
		low_ring0->clr.a = 0.3f;
//		low_ring0->AddToRenderList(RL_GAME_LOW_AURA);

		low_ring1 = enew Ring(kxxwin->LoadTex("boss-b-ring"), 130, 137, -0.035f);
		low_ring1->clr.b = 0.7f;
		low_ring1->clr.a = 0.2f;
	//	low_ring1->AddToRenderList(RL_GAME_LOW_AURA);

		ring0 = enew Ring(kxxwin->LoadTex("boss-b-ring"), 18, 19, 0.035f);
		ring1 = enew Ring(kxxwin->LoadTex("boss-b-ring"), 20, 22, -0.035f);


		pos.x = logic_random_float() * K_GAME_W;
		pos.y = -30;

		//BossBPet * pet = enew BossBPet(pos);
		//pet->pos = pos;
		//pet->ethereal = true;
		//this->AddPet(pet);
		//kxxwin->AddEnemyToList(pet);
		
		SetDark(100.0f, 0.1f);

		BossScript * p;

		float x = K_GAME_XC;
		float y = 100.0f + 50.0f *this->index_in_stage;

		// appear
		p = enew BossBMove(240, x, y + 50);
		p->is_appearing = true;
		AddScript(p);

		// sc
		p = enew BossBMove(120, x, y);
		AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		p = enew BossBSC1();
		AddScript(p);

		// na
	//	p = enew BossBMove(5, 80, y + 50);
		//AddScript(p);

		p = enew BossBNA1();
		AddScript(p);

		//// sc
		//p = enew BossBMove(PS2PF(200), x, 80);
		//AddScript(p);

		//p = enew GatherScript();
		//AddScript(p);

		//p = enew BossBSC2();
		//AddScript(p);

		// sc
		p = enew BossBMove(120, 120, y + 60);
		AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		//p = enew BossBSC3();
		//AddScript(p);


		p = enew BossBNA1();
		AddScript(p);



		// sc
		//p = enew BossBMove(120, x, y);
		//AddScript(p);

		//p = enew GatherScript();
		//AddScript(p);

		//p = enew BossBSC4();
		//AddScript(p);

		// end
		p = enew EndingScript();
		AddScript(p);

		_InitCurrentScript();
	}

	BossB::~BossB()
	{
	}
}
