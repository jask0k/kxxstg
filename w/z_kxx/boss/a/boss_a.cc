
// #include "../config.h"
#include <z_kxx/boss/a/boss_a.h>
#include <z_kxx/boss/a/boss_a_move.h>
#include <z_kxx/boss/a/boss_a_sc1.h>
#include <z_kxx/boss/a/boss_a_sc2.h>
#include <z_kxx/boss/a/boss_a_sc3.h>
#include <z_kxx/boss/a/boss_a_sc4.h>
#include <z_kxx/boss/a/boss_a_sc5.h>
#include <z_kxx/boss/a/boss_a_na1.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
	BossA::BossA(Stage * _stage, int _index_in_stage)
		: Boss("boss-a", _stage, _index_in_stage)
	{
		ani_stand_left = kxxwin->LoadAni(L"boss-a-ani-stand-left");
		ani_stand_right = kxxwin->LoadAni(L"boss-a-ani-stand-right");
		ani_move_left = kxxwin->LoadAni(L"boss-a-ani-move-left");
		ani_move_right = kxxwin->LoadAni(L"boss-a-ani-move-right");
	
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

		low_ring0 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 120, 128, 0.035f);
		low_ring0->clr.g = 0.7f;
		low_ring0->clr.a = 0.3f;
//		low_ring0->AddToRenderList(RL_GAME_LOW_AURA);

		low_ring1 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 130, 137, -0.035f);
		low_ring1->clr.b = 0.7f;
		low_ring1->clr.a = 0.2f;
	//	low_ring1->AddToRenderList(RL_GAME_LOW_AURA);

		ring0 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 18, 19, 0.035f);
		ring1 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 20, 22, -0.035f);

//		showAura0 = false;
//		showAura1 = false;

		pos.x = logic_random_float() * K_GAME_W;
		pos.y = -30;

		BossAPet * pet = enew BossAPet(pos);
		//pet->pos = pos;
		pet->ethereal = true;
		this->AddPet(pet);
		kxxwin->AddEnemyToList(pet);
		
		SetDark(100.0f, 0.1f);

		BossScript * p;

		float x = K_GAME_XC;
		float y = 100.0f + 50.0f *this->index_in_stage;

		// appear
		p = enew BossAMove(120, x, y + 50);
		p->is_appearing = true;
		AddScript(p);


		//p = enew BossAMove(120, x, y);
		//AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		p = enew BossASC5();
		AddScript(p);

		// na
	//	p = enew BossAMove(5, 80, y + 50);
		//AddScript(p);

		p = enew BossANA1();
		AddScript(p);

		//// sc
		//p = enew BossAMove(PS2PF(200), x, 80);
		//AddScript(p);

		//p = enew GatherScript();
		//AddScript(p);

		//p = enew BossASC2();
		//AddScript(p);

		// sc
		p = enew BossAMove(120, x, y + 60);
		AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		p = enew BossASC3();
		AddScript(p);


		p = enew BossANA1();
		AddScript(p);

		// sc
		p = enew BossAMove(120, x, y);
		AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		p = enew BossASC1();
		AddScript(p);

		// sc
		p = enew BossAMove(120, x, y);
		AddScript(p);

		p = enew GatherScript();
		AddScript(p);

		p = enew BossASC4();
		AddScript(p);

		// end
		p = enew EndingScript();
		AddScript(p);

		_InitCurrentScript();
	}

	BossA::~BossA()
	{
	}

	void BossA::BeginFight()
	{
		Boss::BeginFight();
		Sprite * p = pets;
		while(p)
		{
			Enemy * p1 = dynamic_cast<Enemy*>(master);
			if(p1)
			{
				p1->ethereal = false;
			}
			p = p->pet_next;
		}
	}
}
