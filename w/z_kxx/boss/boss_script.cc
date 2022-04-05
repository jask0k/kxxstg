#include <z_kxx/boss/boss_script.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/spark1.h>
#include <z_kxx/util/util_func.h>

namespace e
{
	BossScript::BossScript()
		: next_script(0)
		, boss(0)
		, is_appearing(false)
		, is_fight(false)
		, is_sc(false)
		, is_time_survival(false)
		, init_life(200)
		, max_bonus(POINT_GAIN_ABSORB_SMALL_POINT * 50)
		, time_limit(15*K_LOGIC_FPS)
	{}

	BossScript::~BossScript()
	{}

	void BossScript::Render()
	{}

	bool BossScript::Step()
	{
		return true;
	}

	void BossScript::End()
	{
		if(this->is_fight)
		{
			if(boss->life <= 0 || is_time_survival)
			{
				DropDef drops;
				drops.Set(0, DROP_SMALL_POWER, 1);
				drops.Set(1, DROP_SMALL_POINT, 3, 5);
				drops.Set(2, DROP_NONE);
				kxxwin->AddEnemyDrop(drops, boss->pos);
				AddBonus();
			}

			kxxwin->AddExplosion(boss->pos, 80.0f, boss->hue);
			kxxwin->PlayEnemyDeadSE(boss->pos);
		}
	}

	void BossScript::AddBonus()
	{
		float frac = is_time_survival ? 1.0f : (float(time_limit) * 1.5f - float(boss->script_timer)) / time_limit;
		if(frac > 1.0f)
		{
			frac = 1.0f;
		}
		frac = frac * 0.9f + 0.1f;
		if(boss->is_perfect_round)
		{
			if(is_sc)
			{
				boss->ShowPerfectText();
			}
		}
		else
		{
			frac*= 0.20f;
		}
		float bonus = this->max_bonus * frac * (1.0f + 0.3f * kxxwin->stage->stage_index);
		E_TRACE_LINE(L"[kx] base bonus = " + string(bonus/1000, 1) + L"K.");
		kxxwin->player->AddPoint(boss->pos, bonus, false);
	}

	//MoveScript::MoveScript(float _speed, float _to_x, float _to_y)
	//{
	//	speed_f = _speed;
	//	to.x = _to_x;
	//	to.y = _to_y;
	//}

	//void MoveScript::Begin()
	//{
	//	Vector2 v = to - boss->pos;
	//	speed = v.GetNormal() * speed_f;
	//	time_limit = uint32(v.length() / speed_f + 0.5f);
	//	if(time_limit == 0)
	//	{
	//		time_limit = 1;
	//	}
	//}

	//bool MoveScript::Step()
	//{
	//	boss->pos+= speed;
	//	return true;
	//}

	SCScript::SCScript()
	{
		max_bonus = POINT_GAIN_ABSORB_SMALL_POINT * 100;
		init_life = 400;
		this->is_fight = true;
		this->is_sc = true;
		sc_name = _TT("Unnamed Spell Card");
	}

	NAScript::NAScript()
	{
		this->is_fight = true;
	}

	class GatherAction : public Sprite
	{
		uint32 timer;
		uint32 init_timer;
		Vector2 pos;
		TexRef tex;
	public:
		GatherAction(TexRef _tex, const Vector2 & _pos, uint32 _time)
		{
			tex = _tex;
			pos = _pos;
			init_timer = timer = _time;
		}
		bool Step() override
		{
			if(timer == 0)
			{
				return false;
			}
			timer--;

			if(timer %4 == 1)
			{
				int n = 1 + (int)(4 * CalcFadeInFadeOut(init_timer, timer, 0.3f));
				for(int i=0; i<n; i++)
				{
					float a = CrtRandAngle();
					float cos_a = cos(a);
					float sin_a = sin(a);
					float d = 200 + 300 * frand();
					Vector2 v;
					v.x = pos.x + d * cos_a;
					v.y = pos.y + d * sin_a;

					float speed_f = frand() * 180 + 660;
					float t = d / speed_f;
					float r = (float)tex->W() * 0.5f;
					Spark1 * p = enew Spark1(tex, t, - frand() * PI, frand() * PI, r, r, 0.1f, 0.6f);
					p->pos = v;
					p->speed.x = - PS2PF(speed_f) * cos_a;
					p->speed.y = - PS2PF(speed_f) * sin_a;
					p->blm = BM_ADD;
					p->clr.r = frand() * 0.5f + 0.5f;
					p->clr.g = frand() * 0.5f + 0.5f;
					p->clr.b = frand() * 0.5f + 0.5f;
					kxxwin->AddSparkToList(p, RL_GAME_ENEMY_AURA);
				}
			}
			return true;
		}
	};

	static const int GATHER_TIME_LIMIT = 70 * K_LOGIC_FPS_MUL;
	GatherScript::GatherScript()
	{
		time_limit = GATHER_TIME_LIMIT;
	}

	void GatherScript::Begin()
	{
		kxxwin->PlaySE("gather", boss->pos);
		GatherAction * p = enew GatherAction(kxxwin->LoadTex("gather-star"), boss->pos, GATHER_TIME_LIMIT - 120);
		kxxwin->AddLogicActionToList(p);
		boss->SetDark(600.0f, 0.6f);
	}

	bool GatherScript::Step()
	{
		if(boss->script_timer == time_limit - 120)
		{
			kxxwin->AddBlastWave(boss->pos, true);
			kxxwin->PlaySE("gather-complete", boss->pos);
		//	kxxwin->FreezePlayerShots(boss->pos, 160);
		}
		return true;
	}

	EndingScript::EndingScript()
	{
		speed_f = 4;
	}

	void EndingScript::Begin()
	{
		if(boss->life <= 0)
		{
			speed.x = 0;
			speed.y = 0;
			time_limit = 1;
			kxxwin->AddBossExplosion(boss);
			kxxwin->PlaySE("boss-dead", boss->pos);
		}
		else
		{
			Vector2 v;
			v.x = logic_random_float() * K_GAME_W - boss->pos.x;
			v.y = -40 -  boss->pos.y;
			speed = v.GetNormal() * speed_f;
			time_limit = uint32(v.length() / speed_f + 0.5f);
		}
	}

	bool EndingScript::Step()
	{
		boss->pos+= speed;
		return true;
	}

	void EndingScript::End()
	{
		boss->life = 0;
	}


	MidEndingScript::MidEndingScript()
	{
		state = 0;
		speed_f = 2;
	}

	void MidEndingScript::Begin()
	{
		Vector2 v;
		v.x = logic_random_float() * K_GAME_W - boss->pos.x;
		v.y = -40 -  boss->pos.y;
		speed = v.GetNormal() * speed_f;
		state_timer = uint32(v.length() / speed_f + 0.5f);
		time_limit = boss->mid_boss_timer;
		E_ASSERT(state_timer >= 1);
		E_ASSERT(time_limit >= state_timer);
		boss->ethereal = true;
		OnStateChange();
	}

	bool MidEndingScript::Step()
	{
		if(state == 0)
		{
			boss->pos+= speed;
		}
		if(--state_timer == 0)
		{
			state++;
			state_timer = 5*K_LOGIC_FPS;
			OnStateChange();
		}
		return true;
	}

	void MidEndingScript::End()
	{
		boss->life = 0;
	}

	void MidEndingScript::OnStateChange()
	{
	}

}
