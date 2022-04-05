
// #include "../config.h"
#include <z_kxx/player/player_b.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player_b_shot0.h>
#include <z_kxx/player/player_b_shot1.h>
#include <z_kxx/player/player_b_sc.h>
#include <z_kxx/effect/pass2.h>

namespace e
{
	//static const float MAX_HW = 3.0f;

	PlayerB::PlayerB()
		: Player(1, FRONT_PET)
	{
		shot0 = kxxwin->LoadTex("player-b-shot-0");
		shot0Spark = kxxwin->LoadAni("player-b-shot-0-spark");
		texMasterSpark = kxxwin->LoadTex("player-b-master-spark");
		laser = kxxwin->LoadTex("player-b-laser");
		sc_name_text = _TT("Master Spark");
		hue = 2;
	}

	PlayerB::~PlayerB()
	{
	}

	void PlayerB::Fire()
	{
		if(!CanCastNormalShot() || state.scTimer)
		{
			return;
		}

		const uint32 time = kxxwin->GetLogicTimer();
		int div = 9 * K_LOGIC_FPS_MUL;
		int m = time % div;
		if(time >= state.lastShot0Time + div)
		{
			state.lastShot0Match = m;
		}
		if(m == state.lastShot0Match)
		{
			state.lastShot0Time = time;
			// convert to DPS
			float fixedPower = 20 + 40 * state.power;
			// convert to damage per shot
			fixedPower =  fixedPower * div / K_LOGIC_FPS;

			float dx = 0.20f + SlowFrac() * 0.20f;
			PlayerBShot0 * b = enew PlayerBShot0(this);
			b->power = fixedPower*0.5f;
			b->hsz.y = 8 + fixedPower * 6;
			b->hsz.x = b->hsz.y*0.5f;
			b->pos = this->pos;
			b->pos.y -= 15;
			b->pos.x-= 10;
			b->speed.x = -dx;
			b->speed.y = PS2PF(-1200);
			kxxwin->AddPlayerShotToList(b);

			b = enew PlayerBShot0(this);
			b->power = fixedPower*0.5f;
			b->hsz.y = 8 + fixedPower * 6;
			b->hsz.x = b->hsz.y*0.5f;
			b->pos = this->pos;
			b->pos.y -= 15;
			b->pos.x+= 10;
			b->speed.x = dx;
			b->speed.y = PS2PF(-1200);
			kxxwin->AddPlayerShotToList(b);

			kxxwin->PlaySE("player-shot-0", pos, 0.1f);
		}

		if(state.scTimer || state.petCount == 0)
		{
			return;
		}

		div =  30  * K_LOGIC_FPS_MUL;
		m = time % div;
		if(time >= state.lastShot1Time + div)
		{
			state.lastShot1Match = m;
		}
		if(m == state.lastShot1Match)
		{
			state.lastShot1Time = time;
			float power1 = 40.0f * div / K_LOGIC_FPS / 3;
			// pet shots
			for(int i=0; i<state.petCount; i++)
			{
				Vector2 & v = state.petPosition[i];
				if(kxxwin->pass2)
				{
					kxxwin->pass2->AddExplode(v.x, v.y - 30);
				}
				PlayerBShot1 * b = enew PlayerBShot1(v, laser, petTex, power1);
				kxxwin->AddPlayerShotToList(b, RL_GAME_PLAYER_PET);
			}
		}

	}

	void PlayerB::CastSpellCard()
	{
		E_ASSERT(CanCastSpellCard());
		MasterSpark * p = enew MasterSpark(this);
		kxxwin->AddPlayerSCToList(p);
		kxxwin->PlaySE("spell-card", pos);
		OnSCBegin();
	}

	void PlayerB::RenderPets()
	{
	}

	bool PlayerB::Step()
	{
		Player::Step();
		return true;
	}


	void PlayerB::Render()
	{
		Player::Render();
	}
}
