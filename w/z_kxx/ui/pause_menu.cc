
// #include "../config.h"
#include <z_kxx/ui/player_menu.h>
#include <z_kxx/ui/confirm_menu.h>
#include <z_kxx/ui/pause_menu.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	PauseMenu::PauseMenu(KxxWin * _win)
		: UIBase(_win)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		confirmMenu = 0;
//		tex = 0;
		titleColor = MakeColor(0, 255, 255);
		title = _TT("Pause");
	}

	PauseMenu::~PauseMenu()
	{
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
		if(confirmMenu)
		{
			delete confirmMenu;
		}
	}

	void PauseMenu::RenderBack()
	{
	}

	void PauseMenu::RenderFront()
	{
		// background picture

		// menu color block
		// graphics->BlendOn();
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(0, 0, 0, 0.25f);
		graphics->DrawQuad(menu_x0 - 10, menu_y0 - 10, menu_x0 + menu_w + 10, menu_y0 + menu_h + 10);

		// draw menu item
		float x0 = menu_x0;
		float x1 = x0 + menu_w;
		float y0 = menu_y0;
		float y1 = y0 + item_h;

		graphics->SetTexMode(TM_MODULATE);
		graphics->SetColor(titleColor);
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->SetFont(fontFace); graphics->DrawString(x0, y0, title.c_str(), title.length());
		// graphics->SetTexMode(TextureMode::replace);
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
			graphics->SetColor(item->enabled?  defaultFontColor : disabledFontColor);
			// graphics->SetTexMode(TextureMode::Modulate);
			graphics->SetFont(fontFace); graphics->DrawString( x0, y0, item->text.c_str(), item->text.length());
			// graphics->SetTexMode(TextureMode::replace);
			y0 = y1;
		}

		if(confirmMenu != 0 )
		{	
			//glDisable(GL_ALPHA_TEST);
			// graphics->BlendOn();
			graphics->SetTexMode(TM_DISABLE);
			graphics->SetColor(0, 0, 0, 0.5f);
			graphics->DrawQuad(ui_x0, ui_y0, ui_x1, ui_y1);
			confirmMenu->RenderBack();
			confirmMenu->RenderFront();
		}
	}

	void PauseMenu::CalcMenuSize()
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

	void PauseMenu::CalcMenuPos()
	{
		menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_w) / 2;
		menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
	}

	void PauseMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool PauseMenu::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		if(confirmMenu != 0 )
		{
			return confirmMenu->OnJoystickDown(_joystick, _button);
		}
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
				win->PlayMenuActionSound();
				if(items[activeItem]->needConfirm)
				{
					ShowConfirmMenu();
				}
				else
				{
					items[activeItem]->callback(0);
				}
			}
			break;
		default:
			return false;
		}
		return true;
	}

	void PauseMenu::ShowConfirmMenu()
	{
		confirmMenu = enew ConfirmMenu(win);
		confirmMenu->title = items[activeItem]->text + L"?";

		{
			ConfirmMenu::Item * item = enew ConfirmMenu::Item();
			item->text = _TT("Yes");
			item->enabled = true;
			item->callback = Callback(this, &PauseMenu::OnConfirmYes);
			confirmMenu->items.push_back(item);
		}

		{
			ConfirmMenu::Item * item = enew ConfirmMenu::Item();
			item->text = _TT("No");
			item->enabled = true;
			item->callback = Callback(this, &PauseMenu::OnConfirmNo);
			confirmMenu->items.push_back(item);
		}
		confirmMenu->activeItem = 1;
		confirmMenu->SetBoundRect(ui_x0, ui_y0, ui_x1, ui_y1);
		confirmMenu->fontFace = this->fontFace;
		confirmMenu->CalcMenuSize();
	}

	int PauseMenu::OnConfirmYes(void * _p)
	{
		delete confirmMenu;
		confirmMenu = false;
		items[activeItem]->callback(0);
		return 0;
	}

	int PauseMenu::OnConfirmNo(void * _p)
	{
		delete confirmMenu;
		confirmMenu = false;
		return 0;
	}

}

