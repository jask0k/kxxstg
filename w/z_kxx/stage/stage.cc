
// #include "../config.h"
#include <stdio.h>
#include <z_kxx/stage/stage.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/player/player.h>
#include <z_kxx/globals.h>
#include <z_kxx/stage/kxx_dialog.h>
#include <z_kxx/boss/boss.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/util/util_func.h>
#include <z_kxx/sprite/general_aura.h>
#include <z_kxx/enemy/stage_bonus.h>
#include <z_kxx/sprite/scroll.h>

#pragma warning(disable : 4996)

namespace e
{

	StageBG::StageBG()
		: opacity(false)
		, visible(true)
	{}

	bool StageBG::Step()
	{
		return false;
	}

	void StageBG::Render()
	{}

	void StageBG::FadeOut()
	{}

	Stage::Stage(int _stage_index)
		: stage_index(_stage_index)
	{
		is_test_boss = false;
		wait_pass_state = 0;
		wait_dark_timer = 0;
	}

	Stage::~Stage()
	{
		kxxwin->ClearKxxSpriteLists();
	}

	void Stage::_OnDialog(Stage * _this, int _param0, int _param1)
	{
		_this->dialog = enew KxxDialog(_this, _param0);
	}

	void Stage::_OnDropItem(Stage * _this, int _param0, int _param1)
	{
		if(_param0 > DROP_NONE && _param0 < _DROP_MAX)
		{
			kxxwin->AddDrop(_param0, logic_random_float() * (K_GAME_W - 100) + 50, -15);
		}
		else
		{
			message(L"[kx] (WW) Stage::OnDropItem(): Out of range: " + string(_param0));
		}
	}

	void Stage::_OnTitle(Stage * _this, int _param0, int _param1)
	{
		TexRef fg = kxxwin->LoadTex(L"stage-" + string(_this->human_readable_id()) + L"-title");
		TexRef bg = kxxwin->LoadTex(L"scroll-bg");
		Scroll * p = enew Scroll(fg, bg);
		kxxwin->AddSparkToList(p, RL_GAME_TEXT);
	}

	void Stage::_OnEnemy(Stage * _this, int _param0, int _param1)
	{
		add_enemy(_param0, _param1);
	}

	void Stage::_OnPlayMusic(Stage * _this, int _param0, int _param1)
	{
		kxxwin->PlayBGM(_param0);
	}

	void Stage::_OnStopMusic(Stage * _this, int _param0, int _param1)
	{
		kxxwin->StopBGM();
	}



	void Stage::_OnCreateBoss(Stage * _this, int _param0, int _param1)
	{
		E_ASSERT(_this);
		int index = _param0;
		E_ASSERT(index >= 0);
		if((int)_this->bosses.size() <= index)
		{
			_this->bosses.resize(index+1, 0);
		}
		Boss *& b = _this->bosses[index];
		if(b)
		{
			E_ASSERT(0);
			return;
		}

		_this->OnCreateBoss(index);
	}

	void Stage::_OnBossFight(Stage * _this, int _param0, int _param1)
	{
		E_ASSERT(_this);
		int index = _param0;
		E_ASSERT(index >= 0 && (int)_this->bosses.size() > index);
		if(index >= 0 && (int)_this->bosses.size() > index)
		{
			Boss *& b = _this->bosses[index];
			E_ASSERT(b);
			if(b)
			{
				b->BeginFight();
			}
		}

		_this->OnBossFight(index);
	}

	//void Stage::_OnStageClear(Stage * _this, int _param0, int _param1)
	//{
	//	E_ASSERT(_this);
	//	_this->OnStagePassed();
	//}

	//void Stage::OnStagePassed()
	//{
	//	kxxwin->DarkScreen(2, 1.0f, 0.5f);
	//}

	void Stage::_OnStageBonus(Stage * _this, int _param0, int _param1)
	{
		int life = kxxwin->player->state.life;
		int sc_count = kxxwin->player->state.scCount;

		int total_delay = (life + sc_count) * 60 + 80;
		if(life)
		{
			float w = 20;
			float x0 = K_GAME_XC - w * life * 0.5f;
			TexRef tex = kxxwin->LoadTex(kxxwin->player->short_name + "-ani-f-0");
			for(int i=0; i<life; i++)
			{
				StageBonus * p = enew StageBonus(tex, i * 60 + 1, total_delay);
				p->pos.x = x0 + w * i;
				p->pos.y = 120;
				p->drops.Set(0, DROP_SMALL_POINT, 1, 1);
				p->drops.Set(1, DROP_SMALL_POINT, 18, 18);
				p->drops.Set(2, DROP_TINY_POINT, 20, 20);
				kxxwin->AddEnemyToList(p);
			}
		}

		if(sc_count)
		{
			float w = 20;
			float x0 = K_GAME_XC - w * sc_count * 0.5f;
			TexRef tex = kxxwin->dropTex[DROP_SC];
			for(int i=0; i<sc_count; i++)
			{
				StageBonus * p = enew StageBonus(tex, i * 60 + 1, total_delay);
				p->pos.x = x0 + w * i;
				p->pos.y = 150;
				p->drops.Set(0, DROP_SMALL_POINT, 1, 1);
				p->drops.Set(1, DROP_SMALL_POINT, 7, 7);
				p->drops.Set(2, DROP_TINY_POINT, 10, 10);
				kxxwin->AddEnemyToList(p);
			}
		}
	}


	bool Stage::Step()
	{
		for(int layer = 0; layer<BG_LAYER_COUNT; layer++)
		{
			if(bg[layer])
			{
				if(!bg[layer]->Step())
				{
					bg[layer] = 0;
				}
			}
		}

		if(dialog)
		{
			if(dialog->Step())
			{
				return true;
			}
			else
			{
				dialog.Detach();
			}
		}
		
		bool isBossFight = false;
		for(size_t i = 0; i < bosses.size(); ++i)
		{
			Boss * boss = bosses[i];
			if(boss && boss->is_fighting)
			{
				isBossFight = true;
				break;
			}
		}

		if(isBossFight)
		{
			return true;
		}

		switch(wait_pass_state)
		{
		case 0:
			if(!Timeline::Step())
			{
				wait_pass_state = 1;
			}
			return true;
		case 1:
			if(kxxwin->enemyList.empty()
				&& kxxwin->enemyShotList.empty()
				&& kxxwin->dropList.empty() 
				&& kxxwin->logicStepList.empty()
				&& kxxwin->player->state.scTimer == 0)
			{
				wait_pass_state = 2;
				wait_dark_timer = 1 * K_LOGIC_FPS;
				kxxwin->DarkScreen(2, 1.0f, 0.5f);
			}
			return true;
		case 2:
			if(--wait_dark_timer == 0)
			{
				wait_pass_state = 3;
				return false;
			}
			return true;
		default:
			return false;
		}
	}

	//void Stage::RenderTitle()
	//{
	//	E_ASSERT(titleTimer);
	//	if(titleTimer < TITLE_DURATION)
	//	{
	//		float t = float(titleTimer);
	//		float alpha = 0.6f * CalcFadeInFadeOut(TITLE_DURATION, titleTimer);
	//		// graphics->SetTexMode(TextureMode::Modulate);
	//		float x0 = 110;
	//		float y0 = 160;
	//		graphics->BindTex(titleTex);
	//		graphics->SetColor(1, 1, 1, alpha);
	//		graphics->DrawQuad(0 + x0, 0 + y0, 256 + x0, 128 + y0);
	//		// graphics->SetTexMode(TextureMode::replace);
	//	}
	//}

	bool Stage::Load()
	{
		Path stagePath = Path(L"./") | L"scenario" | string(L"stage-") + string(human_readable_id()) + L".txt";
		FileRef file = kxxwin->OpenResFile( stagePath);
		if(!file)
		{
			message(L"[kx] (WW) Failed to load stage: " +stagePath.GetBaseName());
			return false;
		}

		stringa lineA;
		string line;
		StringArray words;
		//StringArray hms;
		uint32 t = 0;
		while(file->ReadLine(lineA))
		{
			line = string(lineA, CHARSET_UTF8);
			int n = line.find(L'#');
			if(n != -1)
			{
				line = line.substr(0, n);
			}
			line.trim();
			if(line.empty())
			{
				continue;
			}
			words = Split(line, L" \t");
			int wordCount = words.size();
			if(wordCount < 2)
			{
				message(L"[kx] (WW) Bad syntax: " + stagePath.GetBaseName() + L": " + line);
				continue;
			}

			stringa timeText(words[0]);
			int m,s,ds;
			float ft;
			if(3 == sscanf(timeText.c_str(), "%d'%d\"%d", &m, &s, &ds))
			{
				ft = ds*0.1f + s + m * 60;
			}
			else
			{
				message(L"[kx] (WW) Bad syntax: " + stagePath.GetBaseName() + L": " + line);
				continue;
			}

			if(is_test_boss)
			{
				t++;
			}
			else
			{
				t = (int)(ft * K_LOGIC_FPS);
			}

			string & name = words[1];
			int param0 = wordCount > 2 ? words[2].to_int() : 0;
			int param1 = wordCount > 3 ? words[3].to_int() : 0;

			if(name.icompare(L"dialog") ==0)
			{
				E_ASSERT(!words[2].empty());
				if(!is_test_boss)
				{
					this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnDialog, param0);
				}
			}
			else if(name.icompare(L"title") ==0)
			{
				if(!is_test_boss)
				{
					this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnTitle, 0);
				}
			}
			else if(name.icompare(L"enemy") ==0)
			{
				E_ASSERT(!words[2].empty());
				//E_TRACE_LINE("enemy");
				if(!is_test_boss)
				{
					// RegisterEnemy(t, param0);
					RegisterTimeEvents(t, 0, 1, (TIMELINE_HANDLER_FUNC)&_OnEnemy, param0, param1);
				}
			}
			else if(name.icompare(L"create_boss") ==0)
			{
				E_ASSERT(!words[2].empty());
				this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnCreateBoss, param0);
			}
			else if(name.icompare(L"boss_fight") ==0)
			{
				E_ASSERT(!words[2].empty());
				this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnBossFight, param0);
			}
			else if(name.icompare(L"nothing") ==0)
			{
				this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)0, 0);
			}
			else if(name.icompare(L"play_bgm") ==0)
			{
				E_ASSERT(!words[2].empty());
				if(!is_test_boss)
				{
					this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnPlayMusic, param0);
				}
			}
			else if(name.icompare(L"stop_bgm") ==0)
			{
				this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnStopMusic, 0);
			}
			else if(name.icompare(L"drop") ==0)
			{
				this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)&_OnDropItem, param0, param1);
			}
			else
			{
				message(L"[kx] (WW) Bad syntax: " + stagePath.GetBaseName() + L": " + line);
			}
		}

		if(!is_test_boss)
		{
			t+= 1;
			this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)_OnStageBonus, 0);
		}
		
		t+= 1;
//		this->RegisterTimeEvent(t, (TIMELINE_HANDLER_FUNC)_OnStageClear, 0);
		this->RegisterTimeEvent(t + K_LOGIC_FPS, (TIMELINE_HANDLER_FUNC)0, 0);

		return true;
	}

	void Stage::Start() 
	{
		Load();
		Timeline::Start();
	}


	void Stage::GetDebugInfo(string & _info)
	{
		int m, s, ds;
		int ds1 = this->GetTimeTick() * 10 / K_LOGIC_FPS;
		m = ds1 / 600;
		s = (ds1 / 10) % 60;
		ds = ds1  % 10;
		_info = string::format(L"S%d %02d'%02d\"%02d (%02d)", human_readable_id(),  m, s, ds, kxxwin->state.logicTimer/K_LOGIC_FPS);
		Boss * p = this->GetFirstBoss();
		if(p && p->mid_boss_timer)
		{
			int ds1 = p->mid_boss_timer * 10 / K_LOGIC_FPS;
			//m = ds1 / 600;
			s = ds1 / 10;
			ds = ds1  % 10;
			_info+= string::format(L" [%02d\"%02d]", human_readable_id(), s, ds);
		}
	}


	Boss * Stage::GetFirstBoss()
	{
		for(size_t i=0; i<bosses.size(); i++)
		{
			Boss * b = bosses[i];
			if(b)
			{
				return b;
			}
		}
		return 0;
	}

	void Stage::OnMissOrSC()
	{
		if(!bosses.empty())
		{
			int n = bosses.size();
			for(int i=0; i<n; i++)
			{
				 Boss * p = bosses[i];
				 if(p)
				 {
					 p->is_perfect_round = false;
				 }
			}
		}
	}

	void Stage::Render()
	{
		int layer = BG_LAYER_COUNT - 1;
		while(layer && 
			(!bg[layer] || !bg[layer]->visible || !bg[layer]->opacity))
		{
			layer--;
		}

		for(;layer<BG_LAYER_COUNT; layer++)
		{
			if(bg[layer] && bg[layer]->visible)
			{
				bg[layer]->Render();
			}
		}
	}
}
