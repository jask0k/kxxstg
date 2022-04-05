
// #include "../config.h"
#include <z_kxx/ui/top_score_screen.h>
#include <z_kxx/main/kxxwin.h>
#include <nbug/core/env.h>

namespace e
{
	TopScoreScreen::TopScoreScreen(KxxWin * _win, bool _write, int _index)
		: UIBase(_win)
		, write(_write)
		, index(_index)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		titleColor = MakeColor(0, 255, 255);
		title = KxxWin::GetPlayerName(kxxwin->state.playingPlayer) + string(L" ") + KxxWin::GetLevelText(kxxwin->state.level) + string::format(L" TOP %d", K_MAX_TOP_SCORE);
		bgTex = 0;
	}

	TopScoreScreen::~TopScoreScreen()
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

	void TopScoreScreen::RenderBack()
	{
	}

	void TopScoreScreen::RenderFront()
	{
		if(write)
		{
		}
		else
		{
			if(!bgTex)
			{
				bgTex = kxxwin->LoadTex("menu-bg");
			}
			// graphics->BlendOff();
			graphics->SetTexMode(TM_MODULATE);
			graphics->BindTex(bgTex);
			//graphics->SetTextureFilter(false);
			graphics->DrawQuad(0, 0, K_VIEW_W, K_VIEW_H);
			//graphics->SetTextureFilter(true);
		}

		// graphics->SetTexMode(TextureMode::Modulate);
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
		graphics->SetFont(fontFace); graphics->DrawString(x0, y0, title.c_str(), title.length());
		y0 = y1;

		for(uint i=0; i<items.size(); i++)
		{
			float y1 = y0 + item_h;
			Item * item = items[i];
			if(this->activeItem == i)
			{
				graphics->SetTexMode(TM_DISABLE);
				graphics->SetColor(1, 1, 0, 0.25f); 
				graphics->DrawQuad(x0, y0, x1, y1);
			}
			graphics->SetTexMode(TM_MODULATE);
			graphics->SetColor(item->enabled ? defaultFontColor : disabledFontColor);
			graphics->SetFont(fontFace); graphics->DrawString(x0, y0, item->text.c_str(), item->text.length());
			y0 = y1;
		}
		// graphics->SetTexMode(TextureMode::replace);
	}

	void TopScoreScreen::CalcMenuSize()
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

	void TopScoreScreen::CalcMenuPos()
	{
		//menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_w) / 2;
		//menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
		menu_x0 = ui_x0 + 80;
		menu_y0 = ui_y1 - 100 - menu_h;
	}

	void TopScoreScreen::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool TopScoreScreen::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		if(_joystick != 0)
		{
			return false;
		}
		switch(_button)
		{
		case MAPED_BUTTON_UP:
			activeItem--;
			if(activeItem < 0)
			{
				activeItem = items.size() - 1;
			}
			win->PlayMenuSelectSound();
			break;
		case MAPED_BUTTON_DOWN:
			activeItem++;
			if(activeItem >= (int)items.size())
			{
				activeItem = 0;
			}
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

	void TopScoreScreen::Init(const Callback & _callback)
	{
		KxxWin::PersistData *  pd = kxxwin->GetCurrentPersistData();
		for(int i=0; i<K_MAX_TOP_SCORE; i++)
		{
			KxxWin::HighScoreItem & hi = pd->topScores[i];
			Item * item = enew Item();
			item->text = string::format(L"[%d] %10.0f", i, hi.score);
			item->enabled = true;
			item->callback = _callback;
			items.push_back(item);

		}

		{
			Item * item = enew Item();
			item->text = L"Close" ;
			item->enabled = true;
			item->callback = _callback;
			items.push_back(item);
		}
		E_ASSERT(index < (int)items.size() && index >= 0);
		if(index < (int)items.size() && index >= 0)
		{
			activeItem = index;
		}
		else
		{
			activeItem = items.size()-1;
		}
		fontFace = kxxwin->defaultFont;
		SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		CalcMenuSize();
	}
	
	Path TopScoreScreen::GetSelectSlotPath()
	{
		Path folder = Env::GetDataFolder() | "replay";
		return Env::GetDataFolder() | "replay" | string::format(L"%d.kxx01", activeItem+1);
	}

}

