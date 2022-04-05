
// #include "../config.h"
#include <z_kxx/ui/stage_menu.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	StageMenu::StageMenu(KxxWin * _win, bool _isInGame)
		: UIBase(_win)
		, isInGame(_isInGame)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		titleColor = MakeColor(0, 255, 255);
		title = _TT("Select Stage");
		bgTex = 0;
	}

	StageMenu::~StageMenu()
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

	void StageMenu::RenderBack()
	{
	}

	void StageMenu::RenderFront()
	{
		graphics->SetColor(1, 1, 1,1);
		if(isInGame)
		{
			//kxxwin->RenderOffScreenBufferToScreen();
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
			graphics->SetFont(fontFace); graphics->DrawString(x0, y0, item->text.c_str(), item->text.length());
			// graphics->SetTexMode(TextureMode::replace);
			y0 = y1;
		}
	}

	void StageMenu::CalcMenuSize()
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

	void StageMenu::CalcMenuPos()
	{
		//menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_w) / 2;
		//menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
		menu_x0 = ui_x0 + 80;
		menu_y0 = ui_y1 - 100 - menu_h;
	}

	void StageMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool StageMenu::OnJoystickDown(int _joystick, int _button)
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
			if(activeItem>=0 && activeItem<(int)items.size() && items[activeItem]->enabled)
			{
				if(activeItem == items.size() - 1)
				{
					win->PlayMenuCancelSound();
				}
				else
				{
					win->PlayMenuActionSound();
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

	void StageMenu::Init(const Callback & _callback)
	{
		for(int i=0; i<K_STAGE_COUNT; i++)
		{
			StageMenu::Item * item = enew StageMenu::Item();
			if(i == K_STAGE_COUNT-1)
			{
				item->text = _TT("EX Stage");
			}
			else
			{
				item->text = e::Translate(string::format(L"Stage %d", i+1));
			}
			item->enabled = false;
			item->callback = _callback;
			items.push_back(item);
		}

		{
			StageMenu::Item * item = enew StageMenu::Item();
			item->text = _TT("Cancel");
			item->enabled = true;
			item->callback = _callback;
			items.push_back(item);
		}

		activeItem = 0;
		fontFace = kxxwin->defaultFont;
		SetBoundRect(0, 0, K_VIEW_W, K_VIEW_H);
		CalcMenuSize();
	}

}

