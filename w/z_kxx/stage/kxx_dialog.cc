
// #include "../config.h"
#include<z_kxx/stage/kxx_dialog.h>
#include <z_kxx/stage/stage.h>
#include <z_kxx/util/actor.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/boss/boss.h>
#include <z_kxx/util/util_func.h>

namespace e
{
#define AUTO_SWITCH_TIME (5*K_LOGIC_FPS)

	static void ParseVector(Vector2 & _v, const string & _text)
	{
		StringArray a = Split(_text, L",");
		if(a.size() >= 2)
		{
			_v.x = a[0].to_float();
			_v.y = a[1].to_float();
		}
		else
		{
			_v.x = 0;
			_v.y = 0;
		}
	}

	void KxxDialog::LoadItems()
	{
		FileRef file = kxxwin->OpenResFile( Path(L"./") | L"scenario" | kxxwin->player->short_name | string::format(L"dialog-%d-%d.txt", stage->human_readable_id(), dialogID));

	//	FileRef file = FS::OpenFile(path);
		if(!file)
		{
			E_ASSERT(0);
			return;
		}
		// E_SCOPE_RELEASE(file);
		Charset charset = file->ReadBom();
		E_ASSERT(charset == CHARSET_LOCALE || charset == CHARSET_UTF8);

		stringa lineA;
		string line;
		StringArray words;
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
			E_ASSERT(!words[1].empty());
			if(words[0].icompare(L"SAY") == 0 && wordCount >= 3)
			{
				Item * item = enew Item();
				item->action = Say;
				item->actor = 0;
				item->actorShortName = words[1];
				E_ASSERT(words[2].length() == 1);
				item->emotion = words[2].to_int();
				if(wordCount > 3)
				{
					item->text = words[3];
					for(int i=4; i<wordCount; i++)
					{
						item->text+= L" " + words[i];
					}
				}
				else
				{
					item->text = L"...";
				}
				items.push_back(item);
			}
			else if(words[0].icompare(L"NAME") == 0  && wordCount >= 2)
			{
				Item * item = enew Item();
				item->action = ShowName;
				item->actor = 0;
				item->actorShortName = words[1];
				item->emotion = 0;
				items.push_back(item);
			}
			else
			{
				E_ASSERT1(0, L"Unkown action or syntax error: " + line);
			}
		}
	}

	KxxDialog::KxxDialog(Stage * _stage, int _dialogID)
		: dialogID(_dialogID)
	{
		timeTick = 0;
		stage = _stage;
		boxTex = kxxwin->LoadTex("dialog-box");
		leftTex = 0;
		rightTex = 0;
		isLeftSpeeking = false;
		leftItem = 0;
		rightItem = 0;

		leftCurrentPos.x = K_GAME_W * -0.5f;
		leftCurrentPos.y = K_GAME_H * 0.75f;
		leftTargetPos = leftCurrentPos;
		leftTargetAlpha = 1.0f;
		leftCurrentAlpha = 1.0f;
		rightCurrentPos.x = K_GAME_W * 1.5f;
		rightCurrentPos.y = K_GAME_H * 0.75f;
		rightTargetPos = rightCurrentPos;
		rightTargetAlpha = 1.0f;
		rightCurrentAlpha = 1.0f;

		showNameTimer = 0;
		nameTex = 0;

		//isBossMoving = false;
		//moveSpeed = 5.0f;
		LoadItems();
		for(uint i = 0; i < items.size(); i++)
		{
			Item * item = items[i];
			const string & short_name = item->actorShortName;
			ActorMap::iterator it = actorMap.find(short_name);
			if(it == actorMap.end())
			{
				Actor * actor = Actor::Create(short_name);
				actorMap[short_name] = actor;
				item->actor = actor;
			}
			else
			{
				item->actor = it->second;
			}
		}
		fireButtonDown = (kxxwin->GetJoystickState(0) & MAPED_BUTTON_FIRE) ? true : false;
		SwitchToItem(0);

		kxxwin->DamageAllEnemy(1000, false);
		kxxwin->ClearEnemyShots(false, 0);
		kxxwin->AbsorbAllDrops();
	}

	KxxDialog::~KxxDialog()
	{
		for(uint i = 0; i < items.size(); i++)
		{
			delete items[i];
		}

		ActorMap::iterator it = actorMap.begin();
		for(; it != actorMap.end(); ++it)
		{
			delete it->second;
		}
	}

	bool KxxDialog::Step()
	{
		timeTick++;
		if(timeTick % AUTO_SWITCH_TIME == 0) // 10s
		{
			if(!SwitchToItem(GetCurrentItemIndex()))
			{
				return false;
			}
		}
		if(showNameTimer)
		{
			showNameTimer--;
		}
		return true;
	}

	bool KxxDialog::SwitchToItem(int _index)
	{
		E_TRACE_LINE(L"[kx] KxxDialog::SwitchToItem(): " +string(_index));
		E_ASSERT(_index >= 0);
		if(_index >= (int)items.size())
		{
			return false;
		}
		Item * item = items[_index];
		switch(item->action)
		{
		case Say:
			isLeftSpeeking = !item->actor->isEnemy;
			E_ASSERT(item->emotion>= 0 && item->emotion < 8);
			if(isLeftSpeeking)
			{
				leftItem = item;
				leftTex = item->actor->LoadEmotionTex(item->emotion);
				if(leftTex)
				{
					leftTexW = (float)leftTex->W();
					leftTexH = (float)leftTex->H();
				}
				currentContent = item->text;
			}
			else
			{
				rightItem = item;
				rightTex = item->actor->LoadEmotionTex(item->emotion);
				if(rightTex)
				{
					rightTexW = (float)rightTex->W();
					rightTexH = (float)rightTex->H();
				}
				currentContent = item->text;
			}
			break;
		case ShowName:
			nameTex = item->actor->LoadNameTex();
			if(nameTex)
			{
				showNameTimer = 10 * 30;
			}
			SwitchToNextItem(true);
			break;
		default:
			E_ASSERT1(0, L"Invalid action.");
			break;
		}
		return true;
	}

	void KxxDialog::SwitchToNextItem(bool _force)
	{
		uint32 m = timeTick % AUTO_SWITCH_TIME ;
		if(_force || m > S2F(0.3f) && m < AUTO_SWITCH_TIME - S2F(0.3f))
		{
			timeTick = (timeTick / AUTO_SWITCH_TIME) * AUTO_SWITCH_TIME + AUTO_SWITCH_TIME - 1;
		}
	}

	void KxxDialog::CheckJoystick()
	{
		bool newFireButtonDown = (kxxwin->GetJoystickState(0) & MAPED_BUTTON_FIRE) ? true : false;
		if(newFireButtonDown != fireButtonDown)
		{
			fireButtonDown = newFireButtonDown;
			if(fireButtonDown)
			{
				SwitchToNextItem(true);
			}
		}
	}

	void KxxDialog::RenderLeft()
	{
		if(isLeftSpeeking)
		{
			leftTargetAlpha = 1.0f;
			leftTargetPos.x = K_GAME_W * 0.25f;
			leftTargetPos.y = K_GAME_H * 0.75f;
		}
		else
		{
			leftTargetAlpha = 0.5f;
			leftTargetPos.x = K_GAME_W * 0.25f - 20;
			leftTargetPos.y = K_GAME_H * 0.75f + 20;
		}

		if(leftCurrentAlpha != leftTargetAlpha)
		{
			FloatApproach(leftCurrentAlpha, leftTargetAlpha, 0.05f);
		}

		if(leftCurrentPos != leftTargetPos)
		{
			PositionApproach(leftCurrentPos, leftTargetPos, 7);
		}

		float x0 = leftCurrentPos.x - leftTexW * 0.5f;
		float y0 = leftCurrentPos.y - leftTexH * 0.5f;
		float x1 = x0 + leftTexW;
		float y1 = y0 + leftTexH;
	//	float ym = y0 + 128;
			
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->BindTex(leftTex);
		graphics->SetColor(leftCurrentAlpha, leftCurrentAlpha, leftCurrentAlpha, 1.0f); 
		graphics->DrawQuad(x0, y0, x1, y1);
	}

	void KxxDialog::RenderRight()
	{
		bool rightSpeeking = !isLeftSpeeking;
		if(rightSpeeking)
		{
			rightTargetAlpha = 1.0f;
			rightTargetPos.x = K_GAME_W * 0.75f;
			rightTargetPos.y = K_GAME_H * 0.75f;
		}
		else
		{
			rightTargetAlpha = 0.5f;
			rightTargetPos.x = K_GAME_W * 0.75f + 20;
			rightTargetPos.y = K_GAME_H * 0.75f + 20;
		}

		if(rightCurrentAlpha != rightTargetAlpha)
		{
			FloatApproach(rightCurrentAlpha, rightTargetAlpha, 0.05f);
		}

		if(rightCurrentPos != rightTargetPos)
		{
			PositionApproach(rightCurrentPos, rightTargetPos, 7);
		}

		float x0 = rightCurrentPos.x - rightTexW * 0.5f;
		float y0 = rightCurrentPos.y - rightTexH * 0.5f;
		float x1 = x0 + rightTexW;
		float y1 = y0 + rightTexH;
		//float ym = y0 + 128;
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->BindTex(rightTex);
		graphics->SetColor(rightCurrentAlpha, rightCurrentAlpha, rightCurrentAlpha, 1.0f); 
		graphics->DrawQuad(x0, y0, x1, y1);

	}

	void KxxDialog::Render()
	{
		graphics->SetTexMode(TM_MODULATE);

		if(isLeftSpeeking)
		{
			if(rightItem)
			{
				RenderRight();
			}
			if(leftItem)
			{
				RenderLeft();
			}
		}
		else
		{
			if(leftItem)
			{
				RenderLeft();
			}
			if(rightItem)
			{
				RenderRight();
			}
		}


		// graphics->SetTexMode(TextureMode::replace);
		graphics->BindTex(boxTex);
		graphics->DrawQuad(0, K_GAME_H - 100, K_GAME_W, K_GAME_H - 20);
		graphics->SetColor(kxxwin->defaultFontColor);
		graphics->SetFont(kxxwin->defaultFont);
		graphics->DrawString(40, K_GAME_H - 90, K_GAME_W - 40, K_GAME_H - 20,
			currentContent.c_str(), currentContent.length());

		if(showNameTimer!= 0 && nameTex && showNameTimer < 10 * 30)
		{
			float x0 = K_GAME_W - 128;
			float x1 = K_GAME_W - 128 + 64;
			float y0 = K_GAME_H - 100.0f * CalcFadeInFadeOut(10 * 30, showNameTimer);
			float y1 = y0 + 64;
			graphics->BindTex(nameTex);
			graphics->DrawQuad(x0, y0, x1, y1);
		}
	}

	int KxxDialog::GetCurrentItemIndex()
	{
		return timeTick / AUTO_SWITCH_TIME;
	}

}
