
#include <z_kxx/boss/boss_script.h>
#include <z_kxx/boss/boss.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/avatar.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/sprite/sc_name.h>

namespace e
{
#define LOW_RING_SCALE_MAX 1.2f
#define LOW_RING_SCALE_MIN 0.5f
	Boss::Boss(const string & _short_name, Stage * _stage, int _index_in_stage)
		: short_name(_short_name)
		, stage(_stage)
		, index_in_stage(_index_in_stage)
	{
		mid_boss_timer = 0;

		script = 0;
		script_timer = 0;
		is_show_aura = false;
		aura1 = 0;

		low_ring0 = 0;
		low_ring1 = 0;
		low_ring0_scale = LOW_RING_SCALE_MIN;
		low_ring0_scale_sign = true;
		low_ring1_scale = LOW_RING_SCALE_MAX;
		low_ring1_scale_sign = false;

		ring0 = 0;
		ring1 = 0;
		stage->bosses[index_in_stage] = this;
		is_fighting = false;
		ethereal = true;
		display_life = init_life = life = 20;
		is_sc_time = false;
		is_ring_visible = false;
		current_ring_scale = 1.0f;
		pos.x = 0;
		pos.y = 0;
		avatar_tex = kxxwin->LoadTex(stringa(short_name + L"-0").c_str());
		is_perfect_round = true;
		kxxwin->AddEnemyToList(this, RL_GAME_BOSS);

		display_dark_radius = 0;
		target_dark_radius = 0;
		display_dark_alpha = 0;
		target_dark_alpha = 0;

		dark = enew Sprite();
		dark->tex = kxxwin->LoadTex("dark-disc");
		dark->clr.a = 0;
		dark->hsz.x = 1.0f;
		dark->hsz.y = 1.0f;
		dark ->AddToRenderList(RL_GAME_BOTTOM);

		TexRef scNameTex = kxxwin->LoadTex(_short_name + "-sc-name-bg");
		sc_name = enew SCName(true, scNameTex, kxxwin->defaultFont);
	}

	Boss::~Boss()
	{
		delete sc_name;
		delete aura1;
		delete dark;
		delete low_ring0;
		delete low_ring1;
		delete ring0;
		delete ring1;
		//if(gathering)
		//{
		//	delete gathering;
		//}
		E_ASSERT(stage);
		E_ASSERT(stage->bosses[index_in_stage] == this);
		stage->bosses[index_in_stage] = 0;
		while(script)
		{
			BossScript * p = script;
			script = script->next_script;
			delete p;
		}
	}

	void Boss::ShowRing(bool _show)
	{
		if(ring0 || ring1)
		{
			if(_show)
			{
				if(!is_ring_visible)
				{
					if(ring0)
					{
						ring0->AddToRenderList(RL_GAME_ENEMY_AURA);
					}
					if(ring1)
					{
						ring1->AddToRenderList(RL_GAME_ENEMY_AURA);
					}
					is_ring_visible = true;
				}

				current_ring_scale = 1.0f;
				if(ring0)
				{
					ring0->SetScale(current_ring_scale);
				}
				if(ring1)
				{
					ring1->SetScale(current_ring_scale);
				}
			}
			else
			{
				if(is_ring_visible)
				{
					if(ring0)
					{
						ring0->RemoveFromRenderList();
					}
					if(ring1)
					{
						ring1->RemoveFromRenderList();
					}
					is_ring_visible = false;
				}
			}
		}
		else
		{
			is_ring_visible = false;
		}
	}

	void Boss::_InitCurrentScript()
	{
		script->Begin();
		init_life = life = script->init_life;
		is_perfect_round = true;
		is_sc_time = script->is_sc;
		script_timer = 0;
		ethereal = !script->is_fight;
	}

	void Boss::_EndCurrentScript()
	{
		E_ASSERT(script);

		script->End();

		BossScript * p = script;
		script = script->next_script;
		if(script)
		{
			_InitCurrentScript();
		}
		else
		{
			is_fighting = false;
		}
		delete p;
	}

	bool Boss::OnLifeEmpty()
	{
		life = 0;
		_EndCurrentScript();
		return !script;
	}


	void Boss::BeginFight()
	{
		is_fighting = true;
		this->ShowAura(true);
	}


	bool Boss::Step()
	{
		if(mid_boss_timer)
		{
			--mid_boss_timer;
		}
		Enemy::Step();

		if(!script)
		{
			return false;
		}
		dark->ang+= 0.005f;
		dark->pos = pos;
		FloatApproach(display_dark_radius, target_dark_radius, 4);
		FloatApproach(display_dark_alpha, target_dark_alpha, 0.05f);

		dark->scl.x = display_dark_radius;
		dark->scl.y = display_dark_radius;
		dark->clr.a = display_dark_alpha;
		dark->clr.a = display_dark_alpha;

		FloatApproach(display_life, life, 10.0f);

		if(display_life < 0)
		{
			display_life = 0;
		}
		else if(display_life > init_life)
		{
			display_life = init_life;
		}

		E_ASSERT(script->time_limit > 0);
		{
			float alpha;
			if(is_ring_visible)
			{
				alpha = 1.0f - float(script_timer) / script->time_limit;
			}
			else
			{
				alpha = 0.0f;
			}
			if(alpha > 1.0f)
			{
				alpha = 1.0f;
			}
			float target_scale = 20.0f * alpha + 1.0f;
			float delta = (target_scale - this->current_ring_scale) * 0.01f;
			if(delta > 3)
			{
				delta = 3;
			}
			current_ring_scale+= delta;

			if(ring0)
			{
				ring0->clr.a = alpha * 0.3f + 0.15f;
				ring0->pos = pos;
				ring0->SetScale(current_ring_scale);
				ring0->Step();
			}
			if(ring1)
			{
				ring1->clr.a = alpha * 0.2f + 0.10f;
				ring1->pos = pos;
				ring1->SetScale(current_ring_scale);
				ring1->Step();
			}
		}

		if(low_ring0)
		{
			if(low_ring0_scale_sign)
			{
				low_ring0_scale+= 0.001f;
				if(low_ring0_scale >= LOW_RING_SCALE_MAX)
				{
					low_ring0_scale = LOW_RING_SCALE_MAX;
					low_ring0_scale_sign = false;
				}
			}
			else
			{
				low_ring0_scale-= 0.001f;
				if(low_ring0_scale <= LOW_RING_SCALE_MIN)
				{
					low_ring0_scale = LOW_RING_SCALE_MIN;
					low_ring0_scale_sign = true;
				}
			}
			low_ring0->pos = pos;
			low_ring0->SetScale(low_ring0_scale);
			low_ring0->Step();

			aura1->pos = pos;
		}

		if(low_ring1)
		{
			if(low_ring1_scale_sign)
			{
				low_ring1_scale+= 0.002f;
				if(low_ring1_scale >= LOW_RING_SCALE_MAX)
				{
					low_ring1_scale = LOW_RING_SCALE_MAX;
					low_ring1_scale_sign = false;
				}
			}
			else
			{
				low_ring1_scale-= 0.002f;
				if(low_ring1_scale <= LOW_RING_SCALE_MIN)
				{
					low_ring1_scale = LOW_RING_SCALE_MIN;
					low_ring1_scale_sign = true;
				}
			}
			low_ring1->pos = pos;
			low_ring1->SetScale(low_ring1_scale);
			low_ring1->Step();
		}

		sc_name->Step();
		//if(gathering)
		//{
		//	gathering->pos = pos;
		//	gathering->Step();
		//}

		bool step = false;
		if(is_fighting)
		{
			step = true;
		}
		else if(script->is_appearing)
		{
			if(script_timer < script->time_limit-1 || script->next_script &&  script->next_script->is_appearing)
			{
				step = true;
			}
		}

		if(step)
		{
			script_timer++;
			bool b = script->Step();
			if(!b || script_timer >= script->time_limit)
			{
				_EndCurrentScript();
				if(!script)
				{
					return false;
				}
			}
		}
		else if(script->is_appearing)
		{
			script->Step();
		}

		return true;
	}

	void Boss::Render()
	{
		// draw life bar
		float x0 = pos.x - 16;
		float x1 = pos.x + 15;
		float y0 = Top() - 20;
		float y1 = y0 + 4;

		float xc = (x1 - x0) * display_life / init_life + x0;

		graphics->SetTexMode(TM_DISABLE);
		if(is_sc_time && (kxxwin->GetRenderTimer() & 0x04))
		{
			graphics->SetColor(0.7f, 0.3f, 0, 0.5f);
			graphics->DrawQuad(x0, y0, xc, y1);
			graphics->SetColor(0.7f, 0.3f, 0, 0.5f);
			graphics->DrawQuad(xc, y0, x1, y1);
		}
		else
		{
			graphics->SetColor(0, 1, 0, 0.5f);
			graphics->DrawQuad(x0, y0, xc, y1);
			graphics->SetColor(1, 0, 0, 0.5f);
			graphics->DrawQuad(xc, y0, x1, y1);
		}

		graphics->SetTexMode(TM_MODULATE);

		if(script)
		{
			script->Render();
		}
		Enemy::Render();
	}

	void Boss::AddScript(BossScript * _p)
	{
		E_ASSERT(!_p->boss);
		_p->boss = this;
		if(script)
		{
			BossScript * p = script;
			while(p->next_script)
			{
				p = p->next_script;
			}
			p->next_script = _p;
		}
		else
		{
			script = _p;
		}
	}

	void Boss::ShowAvatar()
	{
		E_ASSERT(avatar_tex);
		Avatar * p = enew Avatar();
		p->CreateBoss(avatar_tex);
		kxxwin->AddSparkToList(p, RL_GAME_TEXT);
	}

	void Boss::ShowPerfectText()
	{
		string s = _TT("PERFECT!");
		float w = (float) kxxwin->defaultFont->W(s.c_str(), s.length());
		kxxwin->AddFloatText(pos.x - w*0.5f, pos.y, kxxwin->defaultFont, s,
			120, 1, 1.0f, 0.5f, 0, 1.0f);
	}

	void Boss::ShowSCName(const string & _s)
	{
		if(sc_name->GetRenderListLayer() == -1)
		{
			sc_name->AddToRenderList(RL_GAME_TEXT);
		}
		sc_name->Start(_s);
	}
	void Boss::HideSCName()
	{
		if(sc_name->GetRenderListLayer() != -1)
		{
			sc_name->Explode();
			sc_name->RemoveFromRenderList();
		}
	}
	void Boss::SetDark(float _radius, float _darkness)
	{
		this->target_dark_radius = _radius;
		this->target_dark_alpha = _darkness;
	}

	void Boss::ShowAura(bool _show)
	{
		if(_show)
		{
			if(aura && aura->GetRenderListLayer() == -1)
			{
				aura->AddToRenderList(RL_GAME_LOW_AURA);
			}
			if(aura1 && aura1->GetRenderListLayer() == -1)
			{
				aura1->AddToRenderList(RL_GAME_LOW_AURA);
			}
			if(low_ring0 && low_ring0->GetRenderListLayer() == -1)
			{
				low_ring0->AddToRenderList(RL_GAME_LOW_AURA);
			}
			if(low_ring1 && low_ring1->GetRenderListLayer() == -1)
			{
				low_ring1->AddToRenderList(RL_GAME_LOW_AURA);
			}
		}
		else
		{
			if(aura && aura->GetRenderListLayer() != -1)
			{
				aura->RemoveFromRenderList();
			}
			if(aura1 && aura1->GetRenderListLayer() != -1)
			{
				aura1->RemoveFromRenderList();
			}
			if(low_ring0 && low_ring0->GetRenderListLayer() != -1)
			{
				low_ring0->RemoveFromRenderList();
			}
			if(low_ring1 && low_ring1->GetRenderListLayer() != -1)
			{
				low_ring1->RemoveFromRenderList();
			}
		}
		this->is_show_aura = _show;
	}
}
