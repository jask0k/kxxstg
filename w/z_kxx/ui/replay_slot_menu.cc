
// #include "../config.h"
#include <z_kxx/ui/replay_slot_menu.h>
#include <z_kxx/main/kxxwin.h>
#include <nbug/core/env.h>

namespace e
{
	ReplaySlotMenu::ReplaySlotMenu(KxxWin * _win, bool _save)
		: UIBase(_win)
		, save(_save)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		titleColor = MakeColor(0, 255, 255);
		title = _TT("Select Record");
		bgTex = 0; 
	}

	ReplaySlotMenu::~ReplaySlotMenu()
	{
		//if(bgTex)
		//{
		//	bgTex->Release();
		//}
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
	}

	void ReplaySlotMenu::RenderBack()
	{
	}

	void ReplaySlotMenu::RenderFront()
	{
		if(!bgTex)
		{
			bgTex = kxxwin->LoadTex("menu-bg");
		}
		// graphics->BlendOff();
		graphics->SetTexMode(TM_MODULATE);
		graphics->BindTex(bgTex);
		//graphics->SetTextureFilter(false);
		graphics->SetColor(1, 1, 1, 1);
		graphics->DrawQuad(0, 0, K_VIEW_W, K_VIEW_H);
		//graphics->SetTextureFilter(true);
		

		// menu color block
		// graphics->BlendOn();
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(0, 0, 0, 0.5f);
		graphics->DrawQuad(menu_x0 - 10, menu_y0 - 10, menu_x0 + menu_w + 10, menu_y0 + menu_h + 10);

		// draw menu item
		float x0 = menu_x0;
		float x1 = x0 + menu_w;
		float y0 = menu_y0;
		float y1 = y0 + item_h;

		graphics->SetTexMode(TM_MODULATE);
		graphics->SetColor(titleColor);
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->SetFont(fontFace);
		graphics->DrawString(x0, y0, title.c_str(), title.length());
		// graphics->SetTexMode(TextureMode::replace);
		y0 = y1;

		for(uint i=0; i<items.size(); i++)
		{
			float y1 = y0 + item_h;
			Item * item = items[i];
			if(this->activeItem == i)
			{
				graphics->SetTexMode(TM_DISABLE);
				graphics->SetColor(1, 1, 0, 0.25f); graphics->DrawQuad(x0, y0, x1, y1);
			}
			graphics->SetTexMode(TM_MODULATE);
			graphics->SetColor(item->enabled?  defaultFontColor : disabledFontColor);
			// graphics->SetTexMode(TextureMode::Modulate);
			graphics->SetFont(fontFace);
			graphics->DrawString(x0, y0, item->text.c_str(), item->text.length());
			// graphics->SetTexMode(TextureMode::replace);
			y0 = y1;
		}
	}

	void ReplaySlotMenu::CalcMenuSize()
	{
		menu_w = (float) fontFace->W(title.c_str(), title.length());
		for(uint i=0; i<items.size(); i++)
		{
			Item * item = items[i];
			float w = (float) fontFace->W(item->text.c_str(), item->text.length());
			if(w > menu_w)
			{
				menu_w = w;
			}
		}
		item_h = (float) fontFace->H(); // 
		menu_h = (items.size()+1) * item_h;
		CalcMenuPos();
	}

	void ReplaySlotMenu::CalcMenuPos()
	{
		//menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_w) / 2;
		//menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
		menu_x0 = ui_x0 + 20;
		menu_y0 = ui_y1 - 100 - menu_h;
	}

	void ReplaySlotMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool ReplaySlotMenu::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		if(_joystick != 0)
		{
			return false;
		}
		switch(_button)
		{
		case MAPED_BUTTON_UP:
			do
			{
				activeItem--;
				if(activeItem < 0)
				{
					activeItem = items.size() - 1;
				}
			}while(!items[activeItem]->enabled);
			win->PlayMenuSelectSound();
			break;
		case MAPED_BUTTON_DOWN:
			do
			{
				activeItem++;
				if(activeItem >= (int)items.size())
				{
					activeItem = 0;
				}
			}while(!items[activeItem]->enabled);

			win->PlayMenuSelectSound();
			break;
		case MAPED_BUTTON_FIRE:
			if(items[activeItem]->enabled)
			{
				if(activeItem == items.size() - 1)
				{
					win->PlayMenuCancelSound();
				}
				items[activeItem]->callback(this);
			}
			break;
		case MAPED_BUTTON_SC:
			win->PlayMenuCancelSound();
			activeItem = items.size() - 1;
			items[activeItem]->callback(this);
//			on_cancel(0);
			break;
		default:
			return false;
		}
		return true;
	}

	void ReplaySlotMenu::Init(const Callback & _callback)
	{
		Path folder = Env::GetDataFolder() | "replay";
		FS::CreateFolder(folder);

		int loadFailedCount = 0;
		for(int i=1; i<=K_MAX_REPLAY_SLOT; i++)
		{
			Path file = folder | string::format(L"%d.kxx01", i);
			bool hasFile = FS::IsFile(file);
			KxxWin::ReplayStateInfo replayStageInfo[K_STAGE_COUNT];
			Array<KxxWin::ReplayKey> joystickSequence;
			bool loadSucceed = false;
			if(hasFile)
			{
				loadSucceed = KxxWin::LoadReplay(file, replayStageInfo, K_STAGE_COUNT, &joystickSequence);
				if(!loadSucceed)
				{
					loadFailedCount++;
				}
			}
			if(loadSucceed)
			{
				int player = 0;
				int level = 0;
				for(int j=0; j< K_STAGE_COUNT; j++)
				{
					if(replayStageInfo[j].isReady)
					{
						player = replayStageInfo[j].kxxWinState.playingPlayer;
						level = replayStageInfo[j].kxxWinState.level;
						break;
					}
				}
				Item * item = enew Item();
				//item->text = e::Translate(string::format(L"Record [%d]", i));
				item->text = string::format(L"[%d] ", i) + string(kxxwin->GetLevelText(level)).substr(0, 1);
				float score = 0;
				int lastStage = 0;
				for(int j=0; j< K_STAGE_COUNT; j++)
				{
					if(replayStageInfo[j].isReady)
					{
						lastStage = j;
						if(replayStageInfo[j].kxxWinState.stageHighScore > score)
						{
							score = replayStageInfo[j].kxxWinState.stageHighScore;
						}
					}
				}
				uint32 logicTime = joystickSequence.empty() ? 0 : joystickSequence.back().logicTimer;

				item->text+= L" S" + string(lastStage+1);
				item->text+= string(L" ") + KxxWin::GetPlayerName(player);

				item->text+= string::format(L" (%ds)", logicTime / K_LOGIC_FPS);
				item->text+= string::format(L" %.0f", score);

				item->enabled = save ? true : hasFile;
				item->callback = _callback;
				items.push_back(item);
			}
			else
			{
				Item * item = enew Item();
				item->text = e::Translate(string::format(L"[%d] --- --- --- --- --- --- ", i));
				item->enabled = save ? true : false;
				item->callback = _callback;
				items.push_back(item);
			}
		}

		{
			Item * item = enew Item();
			item->text = save ? _TT("Discard") : _TT("Cancel") ;
			item->enabled = true;
			item->callback = _callback;
			items.push_back(item);
		}

		activeItem = 0;
		fontFace = kxxwin->defaultFont;
		SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		CalcMenuSize();

		if(loadFailedCount)
		{
			kxxwin->ShowNotifyMessage(_TT("1 record loading failed."));
		}
		else if(loadFailedCount > 1)
		{
			kxxwin->ShowNotifyMessage(string(loadFailedCount) + L" " +  _TT("records loading failed."));
		}
	}
	
	Path ReplaySlotMenu::GetSelectSlotPath()
	{
		Path folder = Env::GetDataFolder() | "replay";
		return Env::GetDataFolder() | "replay" | string::format(L"%d.kxx01", activeItem+1);
	}

}

