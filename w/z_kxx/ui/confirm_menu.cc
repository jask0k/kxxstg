
// #include "../config.h"
#include <z_kxx/ui/confirm_menu.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	ConfirmMenu::ConfirmMenu(KxxWin * _win)
		: UIBase(_win)
	{
		activeItem = 0;
		fontFace = 0;
		titleColor = MakeColor(0, 255, 255);
		defaultFontColor = MakeColor(160, 160, 64);
	}

	ConfirmMenu::~ConfirmMenu()
	{
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
	}

	void ConfirmMenu::RenderBack()
	{
	}

	void ConfirmMenu::RenderFront()
	{
		// memu color block
		// graphics->BlendOn();
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(0, 0, 0, 0.75f); 
		graphics->DrawQuad(menu_x0 - 10, menu_y0 - 10, menu_x0 + menu_w + 10, menu_y0 + menu_h + 10);

		// menu item
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
			graphics->SetColor(defaultFontColor);
			graphics->SetFont(fontFace); graphics->DrawString(x0, y0, item->text.c_str(), item->text.length());
			y0 = y1;
		}

	}

	void ConfirmMenu::CalcMenuSize()
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
		item_h = (float) fontFace->H(); 
		menu_h = (items.size()  + 1) * item_h;
		CalcMenuPos();
	}

	void ConfirmMenu::CalcMenuPos()
	{
		menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_w) / 2;
		menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
	}

	void ConfirmMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool ConfirmMenu::OnJoystickDown(int _joystick, int _button)
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
				else
				{
					win->PlayMenuActionSound();
				}
				items[activeItem]->callback(0);
			}
			break;
		case MAPED_BUTTON_SC:
			win->PlayMenuCancelSound();
			activeItem = items.size() - 1;
			items[activeItem]->callback(this);
			break;
		default:
			return false;
		}
		return true;
	}
}

