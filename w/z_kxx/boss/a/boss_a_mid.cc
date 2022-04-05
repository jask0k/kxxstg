
// #include "../config.h"
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/a/boss_a_mid.h>
#include <z_kxx/boss/a/boss_a_move.h>
#include <z_kxx/boss/a/boss_a_pet.h>
#include <z_kxx/boss/a/boss_a_na1.h>
#include <z_kxx/boss/boss_script.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/enemy/enemy_2.h>

namespace e
{
	class BossAMidEndingScript : public MidEndingScript
	{
	protected:
		void OnStateChange() override
		{
			if(state == 0)
			{
				if(boss->pets)
				{
					BossAPet * p = dynamic_cast<BossAPet*>(boss->pets);
					if(p)
					{
						p->ethereal = false;
						p->life = 0;
					}
					boss->RemovePet(boss->pets);
				}
				boss->ShowAura(false);
			}
			else
			{
				add_enemy(2, state % 2);
			}
		}
	};


	BossAMid::BossAMid(Stage * _stage, int _index_in_stage)
		: Boss("boss-a", _stage, _index_in_stage)
	{
		ani_stand_left = kxxwin->LoadAni(L"boss-a-ani-stand-left");
		ani_stand_right = kxxwin->LoadAni(L"boss-a-ani-stand-right");
		ani_move_left = kxxwin->LoadAni(L"boss-a-ani-move-left");
		ani_move_right = kxxwin->LoadAni(L"boss-a-ani-move-right");

		this->mid_boss_timer = 18*K_LOGIC_FPS;
		tex = ani_stand_left.GetTex();
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;


		aura = enew GeneralAura(kxxwin->enemyAuraTex);
		aura->hsz.x = 128;
		aura->hsz.y = 128;
		aura->AddToRenderList(RL_GAME_LOW_AURA);

		aura1 = enew GeneralAura(kxxwin->LoadTex("boss-aura-1"));
		aura1->hsz.x = 100;
		aura1->hsz.y = 100;
		aura1->AddToRenderList(RL_GAME_LOW_AURA);

		low_ring0 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 120, 128, 0.035f);
		low_ring0->clr.g = 0.7f;
		low_ring0->clr.a = 0.3f;
		low_ring0->AddToRenderList(RL_GAME_LOW_AURA);

		low_ring1 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 130, 137, -0.035f);
		low_ring1->clr.b = 0.7f;
		low_ring1->clr.a = 0.2f;
		low_ring1->AddToRenderList(RL_GAME_LOW_AURA);

		ring0 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 18, 19, 0.035f);
		ring1 = enew Ring(kxxwin->LoadTex("boss-a-ring"), 20, 22, -0.035f);

		//showAura0 = false;
		//showAura1 = false;

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

		// na
		p = enew BossANA1();
		AddScript(p);

		// end
		if(_stage->is_test_boss)
		{
			p = enew EndingScript();
			AddScript(p);
		}
		else
		{
			p = enew BossAMidEndingScript();
			AddScript(p);
		}


		_InitCurrentScript();
	}

	BossAMid::~BossAMid()
	{
	}

	void BossAMid::BeginFight()
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
