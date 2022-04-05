
// #include "../config.h"
#include <z_kxx/player/player.h>
#include <z_kxx/globals.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/main/options.h>
#include <z_kxx/stage/stage.h>
#include <z_kxx/sprite/float_text.h>
#include <z_kxx/sprite/ring.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/sprite/avatar.h>
#include <z_kxx/sprite/sc_name.h>

namespace e
{
	class DeadPlayerPet : public Enemy
	{
		uint32 timer;
		float delta;
		bool can_drop;
	public:
		DeadPlayerPet(TexRef _tex, bool _drop)
		{
			tex = _tex;
			can_drop = _drop;
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;
			timer = K_LOGIC_FPS;
			delta = 0.50f / K_LOGIC_FPS;
			ethereal = true;
			init_life = life = 1;
		}
		bool OnLifeEmpty() override
		{
			kxxwin->AddExplosion(pos, 80.0f, hue);
			kxxwin->PlayEnemyDeadSE(pos);

			if(can_drop)
			{
				Vector2 target = {K_GAME_W / 2,  this->pos.y - 600};
				Vector2 delta = target - pos;
				float a0 = delta.Angle();
				delta = target - pos;
				a0 = delta.Angle();
				kxxwin->AddDrop(DROP_PET, pos.x, pos.y, true, a0);
			}

			return true;
		}
		bool Step() override
		{
			if(timer)
			{
				if(--timer == 0)
				{
					ethereal = false;
					life = 0;
				}
				else
				{
					ang = kxxwin->GetRenderTimer() * 0.05f;
					scl.x-= delta;
					scl.y-= delta;
					clr.r-= delta;
					clr.g-= delta;
					clr.a-= delta;
				}
			}
			return true;
		}
	};

	Player::Player(int _id, PetPlaceEnum _petPlace)
		: id(_id)
		, petPlace(_petPlace)
	{
		short_name = L"player-";
		short_name.append(L'a' + id);
		stringa ta = short_name;
		avatar_tex = kxxwin->LoadTex((ta + "-0").c_str());
		//playerAura = kxxwin->LoadTex("player_aura");
		aura = enew SimpleSprite("player-aura");
		gpoint = enew SimpleSprite("player-center");
		gpoint->blm = BM_NORMAL;
		state.extra_card;
		state.curScore = 0;
		state.life = K_INIT_PLAYER_LIFE;
		state.graze = 0;
#ifdef NB_DEBUG
		state.credits = 5;
#else
		state.credits = 2;
#endif

		state.bonus = 1.0f;
		state.extra_bonus = 1.0f;
		state.fastSpeed = K_PLAYER_SPEED_FAST;
		state.slowSpeed = K_PLAYER_SPEED_SLOW;
//		state.isSlow = false;
		state.slowDuration = 0;
		state.scTimer = 0;

		const float size_x = 40.0f / 256.0f;
		const float size_y = 60.0f / 256.0f;
		ani5 = kxxwin->LoadAni5(short_name);
	
		petTex = kxxwin->LoadTex(short_name + L"-pet");

		hsz.x = 20;
		hsz.y = 30;
		ResetState(false);
		this->AddToRenderList(RL_GAME_PLAYER);
		state.addPointAcc = 0;
		state.addPointAccCount = 0;
		TexRef ringTex = kxxwin->LoadTex(short_name + L"-ring");
		scRing = enew Ring(ringTex, 1, 512, PI/60);
		scRing->blm = BM_ADD;

		TexRef scNameTex = kxxwin->LoadTex(short_name + L"-sc-name-bg");
		sc_name = enew SCName(false, scNameTex, kxxwin->defaultFont);

		dark = enew Sprite();
		dark->tex = kxxwin->LoadTex("dark-disc");
		dark->clr.a = 0;
		dark->hsz.x = dark->hsz.y = 512;
		//dark ->AddToRenderList(RL_BOTTOM);
	}

	void Player::SyncEC()
	{
		state.extra_bonus = 1.0f;
		state.fastSpeed = K_PLAYER_SPEED_FAST;
		if(state.extra_card == 0)
		{
			switch(id)
			{
			case 0:
				state.extra_bonus= 1.2f;
				break;
			case 1:
				state.fastSpeed*= 1.2f;
				break;
			default:
				E_ASSERT(0);
				break;
			}
		}
	}

	void Player::ResetState(bool _bornAnimate)
	{
		if(state.slowDuration)
		{
			HideSlow();
		}
		state.slowDuration = 0;
		state.extra_card = 0;

		SyncEC();

		state.pendingDie = false;
		state.petCount = 0;
		state.flyDirection = 0;
#ifdef NB_DEBUG
		state.scCount = 10;
#else
		state.scCount = 1;
#endif
		state.power = 0;
		if(state.scTimer)
		{
			OnSCEnd();
		}
		state.dyingTimer = 0;
		state.deadWaitingTimer=0;
		state.graze = 0;
		for(int i=0; i<MAX_PET; i++)
		{
			state.petPosition[i] = pos;
//			state.petParam[i] = 0;
		}
		state.petCenter = pos;
		state.lastShot0Time = 0;
		state.lastShot0Match = 0;
		state.lastShot1Time = 0;
		state.lastShot1Match = 0;
		if(_bornAnimate)
		{
			state.bornTimer = K_PLAYER_BORN_TIME;
			pos.x = K_GAME_W / 2;
			pos.y = K_INIT_Y + state.fastSpeed * state.bornTimer;
			state.transparentTimer = K_PLAYER_BORN_TIME + K_PLAYER_BORN_TRANSPARENT_TIME;;
		}
		else
		{
			state.bornTimer = 0;
			pos.x = K_GAME_W / 2;
			pos.y = K_INIT_Y;
			state.transparentTimer = K_PLAYER_BORN_TRANSPARENT_TIME;
		}
	}


	Player::~Player()
	{
		delete sc_name;
		delete dark;
		delete scRing;
		delete aura;
		delete gpoint;
		kxxwin->ClearPlayerShots();
	}

	void Player::SetState(const PlayerState & _state)
	{
		this->state = _state;
		if(state.scTimer)
		{
			if(dark->GetRenderListLayer() == -1)
			{
				dark->AddToRenderList(RL_GAME_PLAYER_AURA);
			}
			if(scRing->GetRenderListLayer() == -1)
			{
				scRing->AddToRenderList(RL_GAME_PLAYER_AURA);
			}
			if(sc_name->GetRenderListLayer() == -1)
			{
				sc_name->AddToRenderList(RL_GAME_TEXT);
			}
		}
		else
		{
			if(scRing->GetRenderListLayer() != -1)
			{
				scRing->RemoveFromRenderList();
			}
			if(dark->GetRenderListLayer() != -1)
			{
				dark->RemoveFromRenderList();
			}
			if(sc_name->GetRenderListLayer() != -1)
			{
				sc_name->RemoveFromRenderList();
			}
		}
		if(IsSlow())
		{
			if(aura->GetRenderListLayer() == -1)
			{
				aura->AddToRenderList(RL_GAME_PLAYER_AURA);
			}

			if(gpoint->GetRenderListLayer() == -1)
			{
				gpoint->AddToRenderList(RL_GAME_TOP);
			}
		}
		else
		{
			if(aura->GetRenderListLayer() != -1)
			{
				aura->RemoveFromRenderList();
			}

			if(gpoint->GetRenderListLayer() != -1)
			{
				gpoint->RemoveFromRenderList();
			}
		}

	}

	void Player::Move(uint _joystickState)
	{
		if(!IsControlable())
		{
			return;
		}

		if(_joystickState& MAPED_BUTTON_SLOW)
		{
			if(state.slowDuration < MAX_SLOW_DURATION)
			{
				if(state.slowDuration==0)
				{
					ShowSlow();
				}
				state.slowDuration++;
				float f = SlowFrac();
				aura->clr.a = f;
				aura->scl.x = f;
				aura->scl.y = f;
				gpoint->clr.a = f;
			}
		}
		else
		{
			if(state.slowDuration > 0 )
			{
				state.slowDuration--;
				float f = SlowFrac();
				aura->clr.a = f;
				aura->scl.x = f;
				aura->scl.y = f;
				gpoint->clr.a = f;
				if(state.slowDuration == 0)
				{
					HideSlow();
				}
			}
		}

		E_ASSERT(kxxwin != 0);
	//	const uint state = _joystickState;
		//float playerSpeed;
		float playerSpeed = state.fastSpeed + (state.slowSpeed - state.fastSpeed) * SlowFrac();

		uint direction_button = _joystickState & MAPED_BUTTON_DIRECTION_MASK;
		Vector2 newCenter = this->pos;
		float diagSpeed = playerSpeed * 0.707106781f;
		switch(direction_button)
		{
		case MAPED_BUTTON_LEFT | MAPED_BUTTON_UP | MAPED_BUTTON_RIGHT:
		case MAPED_BUTTON_UP:
			newCenter.y-= playerSpeed;
			state.flyDirection = 0;
			break;
		case MAPED_BUTTON_UP | MAPED_BUTTON_DOWN:
		case MAPED_BUTTON_LEFT | MAPED_BUTTON_RIGHT | MAPED_BUTTON_DOWN:
		case MAPED_BUTTON_DOWN:
			newCenter.y+= playerSpeed;
			state.flyDirection = 0;
			break;
		case MAPED_BUTTON_LEFT | MAPED_BUTTON_RIGHT:
		case MAPED_BUTTON_LEFT | MAPED_BUTTON_UP | MAPED_BUTTON_DOWN:
		case MAPED_BUTTON_LEFT:
			newCenter.x-= playerSpeed;
			state.flyDirection = -1;
			break;
		case MAPED_BUTTON_UP | MAPED_BUTTON_RIGHT | MAPED_BUTTON_DOWN:
		case MAPED_BUTTON_RIGHT:
			newCenter.x+= playerSpeed;
			state.flyDirection = 1;
			break;
		case MAPED_BUTTON_UP | MAPED_BUTTON_LEFT:
			newCenter.x-= diagSpeed;
			newCenter.y-= diagSpeed;
			state.flyDirection = -1;
			break;
		case MAPED_BUTTON_LEFT | MAPED_BUTTON_UP | MAPED_BUTTON_RIGHT | MAPED_BUTTON_DOWN:
		case MAPED_BUTTON_DOWN | MAPED_BUTTON_LEFT:
			newCenter.x-= diagSpeed;
			newCenter.y+= diagSpeed;
			state.flyDirection = -1;
			break;
		case MAPED_BUTTON_UP | MAPED_BUTTON_RIGHT:
			newCenter.x+= diagSpeed;
			newCenter.y-= diagSpeed;
			state.flyDirection = 1;
			break;
		case MAPED_BUTTON_DOWN | MAPED_BUTTON_RIGHT:
			newCenter.x+= diagSpeed;
			newCenter.y+= diagSpeed;
			state.flyDirection = 1;
			break;
		default:
			E_ASSERT(direction_button == 0);
			state.flyDirection = 0;
			break;
		}

		pos = newCenter;
		
		static const int MARGIN_X = 16;
		static const int MARGIN_Y = 26;
		if(this->pos.x < MARGIN_X)
		{
			this->pos.x = MARGIN_X;
		}
		if(this->pos.x > K_GAME_W - MARGIN_X)
		{
			this->pos.x = K_GAME_W - MARGIN_X - 1;
		}
		if(this->pos.y < MARGIN_Y)
		{
			this->pos.y = MARGIN_Y;
		}
		if(this->pos.y > K_GAME_H - MARGIN_Y)
		{
			this->pos.y = K_GAME_H - MARGIN_Y - 1;
		}
	}

	//void Player::RenderShots()
	//{
	//	{
	//		for(auto it = shot0List.begin(); it != shot0List.end(); ++it)
	//		{
	//			(*it)->Render();
	//		}
	//	}
	//	{
	//		for(auto it = shot1List.begin(); it != shot1List.end(); ++it)
	//		{
	//			(*it)->Render();
	//		}
	//	}
	//}



	int Player::TestAbsorbDrop(const Vector2 & _pt, float _r)
	{
		if(!IsControlable())
		{
			return 0;
		}
		float r = (_pt-pos).length() - _r;
		if(r < 12)
		{
			return 2;
		}
		else if(r < 50 && IsSlow() || r < 30)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	void Player::AddPower(float _a)
	{
		E_ASSERT(_a > 0);
		bool power_le_1 = state.power < 1.0f;
		E_ASSERT(power_le_1);
		//int int_power = int(state.power);
		state.power+= _a;
		if(state.power >= 1.0f)
		{
			AddPoint(pos, (state.power - 1.0f) * 200 * POINT_GAIN_ABSORB_TINY_POINT);
			state.power = 1.0f;
			if(power_le_1)
			{
				kxxwin->ClearEnemyShots(pos, 200);
			}
		}
	}


	void Player::SetPetPostion()
	{
#define PET_DELAY_FRAC 0.10f
		if(petPlace == ROTATE_PET)
		{
			E_ASSERT(state.petCount > 0);
			float da = PI * 2 / state.petCount;
			float a = kxxwin->GetLogicTimer() * 0.03f;
			float r = IsSlow() ? 30.0f : 40.0f;
			state.petCenter+= (pos-state.petCenter) * PET_DELAY_FRAC;
			for(int i=0; i<state.petCount; i++)
			{
				state.petPosition[i].x= state.petCenter.x + r * cos(-a);
				state.petPosition[i].y= state.petCenter.y + r * sin(-a);
				a+= da;
			}
		}
		else if(petPlace == BACK_PET)
		{
			static const Vector2 petOffsets[MAX_PET+1][MAX_PET] = 
			{
				{
				
				},
				{
					{ 0.0f, 50.0f},
				},
				{
					{ -30.0f, 40.0f},
					{ +30.0f, 40.0f},
				},
				{
					{ 0.0f, 50.0f},
					{ -30.0f, 40.0f},
					{ +30.0f, 40.0f},
				}
			};
			static const Vector2 petOffsetsSlow[MAX_PET+1][MAX_PET] = 
			{
				{
				
				},
				{
					{ 0.0f, 50.0f},
				},
				{
					{ -20.0f, 40.0f},
					{ +20.0f, 40.0f},
				},
				{
					{ 0.0f, 50.0f},
					{ -20.0f, 40.0f},
					{ +20.0f, 40.0f},
				}
			};
			const Vector2 * petOffsets1 = IsSlow() ? petOffsetsSlow[state.petCount] :  petOffsets[state.petCount];
			for(int i=0; i<state.petCount; i++)
			{
				Vector2 delta = static_cast<const Vector2>(pos + petOffsets1[i]) - state.petPosition[i];
				state.petPosition[i].x+= delta.x * PET_DELAY_FRAC;
				state.petPosition[i].y+= delta.y * PET_DELAY_FRAC;
			}
		}
		else
		{
			static const Vector2 petOffsets[MAX_PET+1][MAX_PET] = 
			{
				{
				
				},
				{
					{ 0.0f, -40.0f},
				},
				{
					{ -24.0f, -32.0f},
					{ +24.0f, -32.0f},
				},
				{
					{ 0.0f, -40.0f},
					{ -24.0f, -32.0f},
					{ +24.0f, -32.0f},
				}
			};
			static const Vector2 petOffsetsSlow[MAX_PET+1][MAX_PET] = 
			{
				{
				
				},
				{
					{ 0.0f, -40.0f},
				},
				{
					{ -16.0f, -32.0f},
					{ +16.0f, -32.0f},
				},
				{
					{ 0.0f, -40.0f},
					{ -16.0f, -32.0f},
					{ +16.0f, -32.0f},
				}
			};
			const Vector2 * petOffsets1 = IsSlow() ? petOffsetsSlow[state.petCount] :  petOffsets[state.petCount];
			for(int i=0; i<state.petCount; i++)
			{
				Vector2 delta = static_cast<const Vector2>(pos + petOffsets1[i]) - state.petPosition[i];
				state.petPosition[i].x+= delta.x * PET_DELAY_FRAC;
				state.petPosition[i].y+= delta.y * PET_DELAY_FRAC;
			}
		}
	}

	void Player::OnSCBegin()
	{
		if(state.scTimer)
		{
			OnSCEnd();
		}
		state.pendingDie = false;
		state.dyingTimer = 0;
		state.scCount--;
		state.scTimer = SPELL_CARD_DURATION;
		state.transparentTimer = SPELL_CARD_DURATION + SPELL_CARD_DURATION_TRANSPARENT_DURATION;
		AddAvatar();
		kxxwin->EarthQuake(SPELL_CARD_EARTH_QUAKE_DURATION_S);
		//kxxwin->DarkScreen(SPELL_CARD_EARTH_QUAKE_DURATION_S);
		kxxwin->AddBlastWave(pos, false);

		E_ASSERT(sc_name->GetRenderListLayer() == -1);
		sc_name->Start(sc_name_text);
		sc_name->AddToRenderList(RL_GAME_TOP);

		E_ASSERT(scRing->GetRenderListLayer() == -1);
		scRing->AddToRenderList(RL_GAME_PLAYER_AURA);

		E_ASSERT(dark->GetRenderListLayer() == -1);
		dark->AddToRenderList(RL_GAME_BOTTOM);

		kxxwin->stage->OnMissOrSC();
	}

	void Player::OnSCEnd()
	{
		state.scTimer = 0;

		E_ASSERT(sc_name->GetRenderListLayer() != -1);
		sc_name->Explode();
		sc_name->RemoveFromRenderList();

		E_ASSERT(scRing->GetRenderListLayer() != -1);
		scRing->RemoveFromRenderList();

		E_ASSERT(dark->GetRenderListLayer() != -1);
		dark->RemoveFromRenderList();

		kxxwin->stage->OnMissOrSC();
	}

	void Player::ShowSlow()
	{
		aura->AddToRenderList(RL_GAME_PLAYER_AURA);
		gpoint->AddToRenderList(RL_GAME_TOP);
	}

	void Player::HideSlow()
	{
		//E_ASSERT(aura->GetRenderListLayer() != -1);
		//E_ASSERT(gpoint->GetRenderListLayer() != -1);
		aura->RemoveFromRenderList();
		gpoint->RemoveFromRenderList();
	}

	bool Player::Step()
	{
		Unit::Step();

		E_ASSERT(state.power<= 1.0f);

		E_ASSERT(state.scTimer <= SPELL_CARD_DURATION);
		if(state.pendingDie)
		{
			Die();
			return true;
		}
		ani5.Step(state.flyDirection);

		if(kxxwin->stage && !kxxwin->stage->dialog)
		{
			if(state.extra_card == 0)
			{
				state.graze*= 0.99989f; 
			}
			else
			{
				state.graze*= 0.99987f; 
			}
		}

		if(state.petCount > 0)
		{
			SetPetPostion();
		}

		if(state.scTimer)
		{
			state.scTimer--;
			if(state.scTimer == 0)
			{
				OnSCEnd();
			}
		}
		E_ASSERT(state.scTimer <= SPELL_CARD_DURATION);

		if(state.transparentTimer)
		{
			state.transparentTimer--;
			float f = state.transparentTimer * 0.1f;
			clr.r = sin(f)*0.4f + 0.5f;
			clr.g = cos(f)*0.4f + 0.5f;
		}
		else
		{
			clr.r = 1.0f;
			clr.g = 1.0f;
		}

		{
			E_ASSERT(state.scTimer <= SPELL_CARD_DURATION);
			if(state.scTimer)
			{
				sc_name->Step();

				scRing->pos = pos;
				float scale = CalcFadeInFadeOut(SPELL_CARD_DURATION, state.scTimer);
				scRing->SetScale(scale);
				scRing->clr.a = scale;
				scRing->Step();

				dark->pos = pos;
				dark->scl.x = dark->scl.y = scale;
				dark->clr.a = scale * 0.7f;
				dark->ang-= 0.005f;
			}
		}
		aura->pos = pos;
		aura->ang+= 0.005f;
		gpoint->pos = pos;

		if(state.bornTimer > 0)
		{
			state.bornTimer--;
			pos.y-= state.fastSpeed;
		}
		if(state.dyingTimer)
		{
			state.dyingTimer--;
			if(state.dyingTimer==0)
			{
				Dead();
			}
		}

		if(state.deadWaitingTimer)
		{
			state.deadWaitingTimer--;
			if(state.deadWaitingTimer == 0)
			{
				Born();
			}
		}

		return true;
	}



	void Player::OnAteDrop(int _type)
	{
		switch(_type)
		{
		case DROP_SMALL_POWER:
			if(IsFullPower())
			{
				this->AddPoint(pos, POINT_GAIN_ABSORB_SMALL_POINT);
			}
			else
			{
				this->AddPower(0.01f);
			}
			break;
		case DROP_SMALL_POINT:
			this->AddPoint(pos, POINT_GAIN_ABSORB_SMALL_POINT);
			break;
		case DROP_TINY_POINT:
			this->AddPoint(pos, POINT_GAIN_ABSORB_TINY_POINT);
			break;
		case DROP_BIG_POWER:
			if(IsFullPower())
			{
				this->AddPoint(pos, POINT_GAIN_ABSORB_SMALL_POINT * 10);
			}
			else
			{
				this->AddPower(0.1f);
			}
			break;
		case DROP_SC:
			state.scCount++;
			break;
		case DROP_PET:
			{
				int sc = state.petCount;
				state.petCount++;
				if(state.petCount > 2)
				{
					state.petCount = 2;
				}
				int sc1 = state.petCount;
				if(sc != sc1)
				{
					for(int i=sc; i<sc1; i++)
					{
						state.petPosition[i] = this->pos;
					}
					kxxwin->PlaySE("add-pet", pos);
				}
			}
			break;
		case DROP_LIFE:
			state.life++;
			kxxwin->PlaySE("add-player", pos);
			break;
		case DROP_FULL:
			{
				if(state.power < 1.0f)
				{
					state.power = 1.0f;
					kxxwin->ClearEnemyShots(pos, 200);
					kxxwin->PlaySE("power-up", pos);
				}

				int pc = state.petCount;
				state.petCount = 2;
				if(pc != state.petCount)
				{
					for(int i=pc; i<state.petCount; i++)
					{
						state.petPosition[i] = this->pos;
					}
					kxxwin->PlaySE("add-pet", pos);
				}
			}
			break;
		case DROP_EC_A:
		case DROP_EC_B:
		case DROP_EC_C:
		case DROP_EC_D:
			{
				int ecid = _type - DROP_EC_A;
				if(state.extra_card != ecid)
				{
					state.extra_card = ecid;
					kxxwin->PlaySE("extra-card", pos);
					SyncEC();
				}
			}
			break;
		default:
			message(L"[kx] (WW) unimpletmented drop type=" + string(_type));
			break;
		}
	}
	
	void Player::Die()
	{
		E_ASSERT(!state.scTimer);
		E_ASSERT(!state.transparentTimer);

		kxxwin->stage->OnMissOrSC();
		E_ASSERT(state.pendingDie);
		state.pendingDie = false;
		if(state.dyingTimer == 0)
		{
			// critical time
			state.transparentTimer = 0x7fffffff;
			state.dyingTimer = S2F(0.25f);
			// critical punish
			state.graze = 0; 
			kxxwin->DarkScreen(0.5f);
			kxxwin->AddBlastWave(pos, true);
			kxxwin->PlaySE("player-dead", pos);
		}
	}

	void Player::ThrowDeadDrops()
	{
		Vector2 target = {K_GAME_W / 2,  this->pos.y - 600};
		Vector2 delta = target - pos;
		float a0 = delta.Angle();
		
		float powerDrop = state.power * 0.50f;
		// drop some small p event if no power
		if(powerDrop < 0.05f)
		{
			powerDrop = 0.05f;
		}

		// drop at least 1 small p
		int drop2Count = 1;
		powerDrop-=0.01f;

		// drop one pet if condition match
		int drop0[5];
		int drop0Count = 0;
		if(state.life < 0)
		{
			drop0[drop0Count++] = DROP_FULL;
		}

		//if(state.extra_card)
	//	{
	//		drop0[drop0Count++] = DROP_EC_A + state.extra_card;
	//	}

		// drop big p
		int drop1Count = int(powerDrop * 10.0f);
		// calc remain small p
		drop2Count+=  int((powerDrop - drop1Count*0.1f) / 0.01f);
		// total drop
		int n = drop0Count + drop1Count + drop2Count;
		if(n > 0)
		{
			float side = -1;
			float da = PI * 0.2f / n;
			for(int i=0; i<drop2Count; i++)
			{
				int m = n / 2;
				kxxwin->AddDrop(DROP_SMALL_POWER, pos.x, pos.y, true, a0 + side * m * da);
				n--;
				side = -side;
			}		
			
			for(int i=0; i<drop1Count; i++)
			{
				int m = n / 2;
				kxxwin->AddDrop(DROP_BIG_POWER, pos.x, pos.y, true, a0 + side * m * da);
				n--;
				side = -side;
			}

			for(int i=0; i<drop0Count; i++)
			{
				int m = n / 2;
				kxxwin->AddDrop(drop0[i], pos.x, pos.y, true, a0 + side * m * da);
				n--;
				side = -side;
			}
		}

			kxxwin->AddDrop(DROP_EC_A + state.extra_card, pos.x, pos.y, true, -0.5f * PI);

		//if(state.petCount)
		//{
		//	Vector2 target = {K_GAME_W / 2,  this->pos.y - 600};
		//	Vector2 delta = target - pos;
		//	float a0 = delta.Angle();
		//	Vector2 &v = state.petPosition[0];
		//	delta = target - v;
		//	a0 = delta.Angle();
		//	kxxwin->AddExplosion(v, 80);
		//	kxxwin->AddDrop(DROP_PET, v.x, v.y, true, a0);
		//}
	}

	void Player::Dead()
	{
		E_ASSERT(!state.scTimer);
//		E_ASSERT(!state.transparentTimer);
		state.life--;
		kxxwin->AddBossExplosion(this);
		kxxwin->DamageAllEnemy(3, false);
		ThrowDeadDrops();
		for(int i=0; i< state.petCount; i++)
		{
			DeadPlayerPet * p = enew DeadPlayerPet(petTex, i == 0);
			p->pos = state.petPosition[i];
			kxxwin->AddEnemyToList(p);
		}
		//kxxwin->ClearEnemyShots(0);
		kxxwin->FreeAbsorbingDrops();
		pos.x = K_GAME_W / 2;
		pos.y = K_INIT_Y + state.fastSpeed * K_PLAYER_BORN_TIME;
		state.deadWaitingTimer = S2F(1.0f);
		state.dyingTimer = 0;
		state.petCount = 0;
	}

	void Player::Born()
	{
		if(state.life >= 0)
		{
			ResetState(true);
		}
		else if(kxxwin->isReplaying)
		{

			kxxwin->isReplaying = false;
			kxxwin->OnQuitReplay();
		}
		else 
		{
			kxxwin->SwitchToContinueMenu();
		}
	}


	void Player::Continue()
	{
		kxxwin->state.onceContinued = true;

		ResetState(true);
		state.life = K_INIT_PLAYER_LIFE;

		//state.power = 0.5;
		//state.scCount = 2;

		state.bonus*= 0.75f;
		state.credits--;
	}

	void Player::AddPoint(const Vector2 & _pt, float _i, bool _frequently)
	{

		float add = _i * GetTotalBonus();
		state.curScore+=add;
		if(!kxxwin->isPractice && state.curScore > kxxwin->highScoreCopy)
		{
			kxxwin->highScoreCopy = state.curScore;
		}
		E_ASSERT(add >= 0);

		int display = int(add+0.5f);
		if(_frequently)
		{
			state.addPointAcc+= add;
			state.addPointAccCount++;
			if(state.addPointAccCount >= 15 || (state.addPointAccPos-_pt).length() > 32)
			{
				display = int(state.addPointAcc+0.5f);
				state.addPointAcc = 0;
				state.addPointAccCount = 0;
				state.addPointAccPos = _pt;
			}
			else
			{
				return;
			}
		}
		
		if(display >0 )
		{
			FloatText * ft = enew FloatText;
			
			float df = (display - 50)  / 200.0f;
			if(df > 1.0f)
			{
				df = 1.0f;
			}
			else if(df <0)
			{
				df = 0;
			}

			ft->clr.r = 0.39f + uint8(0.60f * df);
			ft->clr.g = 0.39f + uint8(0.47f * df);
			ft->clr.b = 0.78f - uint8(0.78f * df);
			ft->clr.a = 1.0f;
		
			ft->text = string::format(L"+%d", display);
			float w = (float)  kxxwin->smallFont->W(ft->text.c_str(), ft->text.length());
			ft->pos = _pt;
			ft->pos.x-= w*0.5f;

			ft->pos.x += frand() * 20 - 10;
			ft->pos.y += frand() * 20 - 10;

			ft->pos.x = (float)(int)ft->pos.x;
			ft->pos.y = (float)(int)ft->pos.y;
			ft->font = kxxwin->smallFont;
			kxxwin->AddSparkToList(ft, RL_GAME_TEXT);
		}

	}

	void Player::Render()
	{
		RenderPets();
		tex = ani5.GetTex();

		Sprite::Render();
	}

	//int Player::state.petCount const
	//{
	//	int sc = int(state.petCount);
	//	return sc > MAX_PET ? MAX_PET : sc;
	//}

	bool Player::IsControlable() const
	{
		return !(state.dyingTimer || state.bornTimer || state.deadWaitingTimer);
	}

	bool Player::CanCastNormalShot() const
	{
		return IsControlable() && kxxwin->stage->wait_dark_timer == 0;
	}

	bool Player::CanCastSpellCard() const
	{
		return state.scTimer < SPELL_CARD_RECAST_LINE 
			&& state.scCount >= 1.0f 
			&& state.bornTimer == 0 
			&& state.deadWaitingTimer==0
			&& kxxwin->stage->wait_dark_timer == 0;
	}

	bool Player::BlockEnemyShot(EnemyShot * _p)
	{
		return _p->Collide(pos, 16 + K_GRAZE_RADIUS);
	}

	
	float Player::GetTotalBonus() const
	{
		return (state.graze+1) * state.bonus * state.extra_bonus;
	}

	void Player::OnGraze()
	{
		state.graze+= 0.050f;
	}

	void Player::AddAvatar()
	{
		Avatar * p = enew Avatar();
		p->CreatePlayer(avatar_tex);
		kxxwin->AddSparkToList(p, RL_GAME_TEXT);
	}

	bool Player::IsCollideWith(Enemy * _enemy)
	{
		if(state.transparentTimer)
		{
			return false;
		}
		
		float a = (pos - _enemy->pos).LengthSquared();
		float b = (_enemy->GetCollisionRadius() + K_GPOINT_RADIUS) * 0.8f;
		return a < b * b;
	}

	void Player::RenderDebug()
	{
		RGBA color = {0, 0.75, 1, 1};
		float r = K_GPOINT_RADIUS;
		graphics->DrawEllipse(pos.x, pos.y, ang, r, r, color, false);

		if(IsSlow())
		{
			RGBA color1 = {1, 0.75, 0, 1};
			r = K_GPOINT_RADIUS + K_GRAZE_RADIUS;
			graphics->DrawEllipse(pos.x, pos.y, ang, r, r, color1, false);
		}
	}

}
