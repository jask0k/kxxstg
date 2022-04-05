
// #include "../config.h"
#include <z_kxx/player/player_a.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player_a_shot1.h>
#include <z_kxx/player/player_a_shot0.h>
#include <z_kxx/player/player_a_sc.h>

namespace e
{
	PlayerA::PlayerA()
		: Player(0, BACK_PET)
	{
		state.smooth_da = PS2PF(PI * 0.25f);	

		yyyTex = kxxwin->LoadTex("player-a-yyy");
		shot0 = kxxwin->LoadTex("player-a-shot-0");
		shot1 = kxxwin->LoadTex("player-a-shot-1");
		shot0Spark = kxxwin->LoadAni("player-a-shot-0-spark");
		shot1Spark = kxxwin->LoadAni("player-a-shot-1-spark");

		sc_name_text = _TT("Unnamed Spell Card");
		hue = 1;
	}

	PlayerA::~PlayerA()
	{
	}

	void PlayerA::Fire()
	{
		if(!CanCastNormalShot())
		{
			return;
		}

		const uint32 time = kxxwin->GetLogicTimer();
		int div = 8 * K_LOGIC_FPS_MUL;
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
			if(state.scTimer)
			{
				fixedPower*= 1.5f;
			}
			// convert to damage per shot
			fixedPower = fixedPower * div / K_LOGIC_FPS;
			// track count
			int n = (int)(fixedPower + 0.01f);
			// power per track
			float power1 = 1.0f;

			float half_n = float(n) / 2.0f;
			float da = PI * 0.035f;
			if(IsSlow())
			{
				da*= 0.65f;
			}
			state.smooth_da+= angle_limit(da - state.smooth_da, 0.02f);
			float a0 = -(PI * 0.5f) - half_n * state.smooth_da;
			float a1 = -(PI * 0.5f) + half_n * state.smooth_da + 0.001f;
			for(float a = a0 ; a <= a1; a+= state.smooth_da)
			{
				PlayerAShot0 * b = enew PlayerAShot0(this);
				b->hsz.y = 30;
				b->hsz.x = 10;
				b->pos = this->pos;
				b->clr.a = 1.0f;
				b->speed.x = PS2PF(750) * cos(a);;
				b->speed.y = PS2PF(750) * sin(a);
				b->pos.x+=b->speed.x * 2;
				b->RotateNorthToVectorDirection(b->speed);
				b->power = power1;
				kxxwin->AddPlayerShotToList(b);
			}
			kxxwin->PlaySE("player-shot-0", pos, 0.1f);

		}

		if(state.petCount && !kxxwin->enemyList.empty())
		{
			div = 5 * K_LOGIC_FPS_MUL;
			m = time % div;
			if(time >= state.lastShot1Time + div)
			{
				state.lastShot1Match = m;
			}
			if(m == state.lastShot1Match )
			{
				state.lastShot1Time = time;
				float power1 = 40.0f * div / K_LOGIC_FPS / 3;

				Enemy * enemy[4];
				int n = kxxwin->FindSomeNearEnemy(enemy, state.petCount, pos);
				for(int i = n; i <= state.petCount; i++)
				{
					enemy[i] = enemy[0];
				}

				for(int i=0; i<state.petCount; i++)
				{
					Vector2 & v = state.petPosition[i];
					float target_angle = enemy[i] ? (enemy[i]->pos - v).Angle() : -0.5f * PI;
					PlayerAShot1 * b = enew PlayerAShot1(this, v, target_angle);
					b->power = power1;
					if(enemy[i])
					{
						enemy[i]->AddFollower(b);
					}
					kxxwin->AddPlayerShotToList(b);
				}
			}
		}
	}

	void PlayerA::CastSpellCard()
	{
		E_ASSERT(CanCastSpellCard());
		PlayerAYYYSC * p = enew PlayerAYYYSC(pos, yyyTex);
		kxxwin->AddPlayerSCToList(p);
		kxxwin->PlaySE("spell-card", pos);
		OnSCBegin();
	}

	void PlayerA::RenderPets()
	{
		if(state.petCount > 0)
		{
			graphics->SetBlendMode(BM_NORMAL);
			graphics->SetColor(clr);
			for(int i=0; i<state.petCount; i++)
			{
				Vector2 & v = state.petPosition[i];
				graphics->PushMatrix();
				graphics->TranslateMatrix(v.x, v.y, 0);
				float a = kxxwin->GetRenderTimer() * 0.05f;
				graphics->RotateMatrix(a, 0, 0, 1);
				graphics->BindTex(petTex);
				graphics->DrawQuad(-10, -10, 10, 10);
				graphics->PopMatrix();
			}
		}
	}

	bool PlayerA::Step()
	{
		Player::Step();
		return true;
	}

}
