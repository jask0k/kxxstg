
// #include "../config.h"
#include <z_kxx/ui/options_menu.h>
#include <z_kxx/main/kxxwin.h>
//#include "OldGraphics.h"

namespace e
{
	const float OptionsMenu::spacing = 10;

	OptionsMenu::OptionsMenu(KxxWin * _win)
		: UIBase(_win)
	{
		require_restart = false;
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		titleColor = MakeColor(0, 255, 255);
		bgTex = kxxwin->LoadTex("menu-bg");
		title = _TT("Options");
	}

	OptionsMenu::~OptionsMenu()
	{
		//if(bgTex)
		//{
		//	bgTex->Release();
		//}
		if(require_restart)
		{
			win->ShowNotifyMessage(_TT("Restart the game to complete your changes."), 3, 4);
		}
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
	}

	void OptionsMenu::RenderBack()
	{
	}

	void OptionsMenu::RenderFront()
	{
		// background picture
		// graphics->BlendOff();
		graphics->SetColor(0xffffffff);
		graphics->SetTexMode(TM_MODULATE);
		graphics->BindTex(bgTex);
	//	graphics->SetTextureFilter(false);
		graphics->DrawQuad(0, 0, K_VIEW_W, K_VIEW_H);
	//	graphics->SetTextureFilter(true);
		// menu color block
		// graphics->BlendOn();
		graphics->SetTexMode(TM_DISABLE);
		graphics->SetColor(0, 0, 0, 0.5);
		graphics->DrawQuad(menu_x0 - 10, menu_y0 - 10, menu_x0 + menu_left_w + menu_right_w + 10, menu_y0 + menu_h + 10);

		// draw menu item
		float x0 = menu_x0;
		float x1 = x0 + menu_left_w + menu_right_w;
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
				graphics->SetColor(1, 1, 0, 0.25f); graphics->DrawQuad(x0, y0, x1, y1);
			}

			// draw label
			float textWidth = (float) fontFace->W(item->text.c_str(), item->text.length());
			graphics->SetTexMode(TM_MODULATE);
			// graphics->SetTexMode(TextureMode::Modulate);
			if(item->IsButton())
			{
				graphics->SetColor(defaultFontColor);
				graphics->SetFont(fontFace); graphics->DrawString(x0 + (menu_left_w + menu_right_w - textWidth) / 2, y0, item->text.c_str(), item->text.length());
			}
			else
			{
				graphics->SetColor(defaultFontColor);
				graphics->SetFont(fontFace); graphics->DrawString(x0 + menu_left_w - textWidth, y0, item->text.c_str(), item->text.length());
			}
			// graphics->SetTexMode(TextureMode::replace);
			// draw options
			float xx0 = x0 + menu_left_w + spacing;
			for(uint j=0; j<item->options.size(); j++)
			{
				string & text = item->options[j];
				textWidth = (float) fontFace->W(text.c_str(), text.length());
				float xx1 = xx0 + textWidth;
				if(item->activeOption == j)
				{
					graphics->SetTexMode(TM_DISABLE);
					if(this->activeItem == i)
					{
						graphics->SetColor(1, 1, 0, 0.25f);
						graphics->DrawQuad(xx0, y0, xx1, y1);
					}
					else
					{
						graphics->SetColor(0.5f, 0.5f, 0, 0.25f);
						graphics->DrawQuad(xx0, y0, xx1, y1);
					}

				}
				graphics->SetTexMode(TM_MODULATE);
				graphics->SetColor(defaultFontColor);
				// graphics->SetTexMode(TextureMode::Modulate);
				graphics->SetFont(fontFace); graphics->DrawString(xx0, y0, text.c_str(),text.length());
				// graphics->SetTexMode(TextureMode::replace);
				xx0+= textWidth + spacing;
			}

			y0 = y1;
		}

		//glPopAttrib();
	}

	void OptionsMenu::CalcMenuSize()
	{
		menu_left_w = 0;
		menu_right_w = 0;
		for(uint i=0; i<items.size(); i++)
		{
			Item * item = items[i];
			// left
			float w = (float) fontFace->W(item->text.c_str(), item->text.length());
			if(w > menu_left_w)
			{
				menu_left_w = w;
			}
			// right
			w = 0;
			for(uint j=0; j<item->options.size(); j++)
			{
				string & text = item->options[j];
				w+= spacing + (float) fontFace->W(text.c_str(), text.length());
			}
			if(w > menu_right_w)
			{
				menu_right_w = w;
			}
		}
		item_h = (float) fontFace->H(); // 
		menu_h = (items.size()+1) * item_h;
		CalcMenuPos();
	}

	void OptionsMenu::CalcMenuPos()
	{
		menu_x0 = ui_x0 + ((ui_x1 - ui_x0) - menu_left_w - menu_right_w) / 2;
		menu_y0 = ui_y0 + ((ui_y1 - ui_y0) - menu_h) / 2;
	}

	void OptionsMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool OptionsMenu::OnJoystickDown(int _joystick, int _button)
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
		case MAPED_BUTTON_LEFT:
			{
				Item * item = items[activeItem];
				if(!item->IsButton())
				{
					item->activeOption--;
					if(item->activeOption < 0)
					{
						item->activeOption = 0;
					}
					item->callback(item);
					if(item->require_restart)
					{
						this->require_restart = true;
					}
				}
			}
			break;
		case MAPED_BUTTON_RIGHT:
			{
				Item * item = items[activeItem];
				if(!item->IsButton())
				{
					item->activeOption++;
					if(item->activeOption >= (int)item->options.size())
					{
						item->activeOption = item->options.size() - 1;
					}
					item->callback(item);
					if(item->require_restart)
					{
						this->require_restart = true;
					}
				}
			}
			break;
		case MAPED_BUTTON_FIRE:
			if(items[activeItem]->IsButton())
			{
				if(activeItem == items.size() - 1)
				{
					win->PlayMenuCancelSound();
				}
				else
				{
					win->PlayMenuActionSound();
				}
				items[activeItem]->callback(items[activeItem]);
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

