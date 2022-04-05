
// #include "../config.h"
#include <z_kxx/ui/player_menu.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/static_pic.h>
#include <z_kxx/sprite/dynamic_pic.h>
#include <z_kxx/main/options.h>

namespace e
{
#define S_INACTIVE 0
#define S_FADE_IN 1
#define S_ACTIVE 2
#define S_FADE_OUT 3

	PlayerMenu::PlayerMenu(KxxWin * _win)
		: UIBase(_win)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		titleColor = MakeColor(0, 255, 255);
		title = _TT("Characters");
		bg = enew StaticImage("menu-bg", 0, 0, K_VIEW_W, K_VIEW_H);
		bg->AddToRenderList(RL_UI_BACK_0);
		level = win->options->level;
		if(kxxwin->isExStart)
		{
			ex_stage_tex = kxxwin->LoadTex("level-ex");
		}
		else
		{
			for(int i=0; i< K_LEVEL_COUNT; i++)
			{
				level_tex[i] = kxxwin->LoadTex("level-" + stringa(i));
			}
		}
	}

	PlayerMenu::~PlayerMenu()
	{
		delete bg;
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
		//for(int i=0; i<4; i++)
		//{
		//	E_SAFE_RELEASE(level_tex[i]);
		//}
	}

	void PlayerMenu::RenderBack()
	{
	}

	void PlayerMenu::RenderFront()
	{
		// background picture
		// graphics->BlendOff();
		graphics->SetTexMode(TM_MODULATE);

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
			graphics->SetTexMode(TM_MODULATE);
			graphics->SetColor(item->enabled?  defaultFontColor : disabledFontColor);
			// graphics->SetTexMode(TextureMode::Modulate);
			graphics->SetFont(fontFace); graphics->DrawString(x0, y0, item->text.c_str(), item->text.length());
			// graphics->SetTexMode(TextureMode::replace);
			y0 = y1;
		}

		{
			graphics->SetColor(0xffffffff);
			TexRef tex = kxxwin->isExStart ? ex_stage_tex : level_tex[level];
			float x = 200.0f;
			float y = K_VIEW_H - 80.0f;
			float w, h;
			if(tex)
			{
				w = (float)tex->W();
				h = (float)tex->H();
			}
			else
			{
				w = 80;
				h = 20;
			}
			graphics->BindTex(tex);
			graphics->DrawQuad(x, y, x+w, y+h);
		}
	}

	void PlayerMenu::SwitchToPlayer(int _i)
	{
		int oldIndex = this->activeItem;
		activeItem = _i;
		Item * oldItem = items[oldIndex];
		if(oldItem->body)
		{
			oldItem->body->FadeOut();
			oldItem->body = 0;
		}
		Item * newItem = items[_i];
		if(_i < 3)
		{
			E_ASSERT(newItem->body == 0);
			string name = KxxWin::GetPlayerShortName(_i);
			name = name + "-0";
			TexRef t = kxxwin->LoadTex(name);
			if(t)
			{

				newItem->body = enew DynamicImage(name, K_VIEW_W - 20 - (float)t->W(), K_VIEW_H - 20 - (float)t->H(), K_VIEW_W - 20, K_VIEW_H - 20);
				newItem->body->method = DynamicImage::FlipH;
				newItem->body->SetTimer(20, 0, 20, false);
				kxxwin->AddUISpriteToList(newItem->body, RL_UI_FRONT_1);
//				t->Release();
			}
		}
	}

	void PlayerMenu::CalcMenuSize()
	{
		menu_w = 0;
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

	void PlayerMenu::CalcMenuPos()
	{
		menu_x0 = ui_x0 + 8;
		menu_y0 = ui_y1 - 8 - menu_h;
	}

	void PlayerMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuPos();
	}

	bool PlayerMenu::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		if(_joystick != 0)
		{
			return false;
		}
		switch(_button)
		{
		case MAPED_BUTTON_LEFT:
			if(!kxxwin->isExStart)
			{
				level--;
				if(level < 0)
				{
					level = K_LEVEL_COUNT-1;
				}
				win->PlayMenuSelectSound();
			}
			break;
		case MAPED_BUTTON_RIGHT:
			if(!kxxwin->isExStart)
			{
				level++;
				if(level >= K_LEVEL_COUNT)
				{
					level = 0;
				}
				win->PlayMenuSelectSound();
			}
			break;
		case MAPED_BUTTON_UP:
			{
				int n = activeItem-1;
				if(n < 0)
				{
					n = items.size() - 1;
				}
				SwitchToPlayer(n);
			}
			win->PlayMenuSelectSound();
			break;
		case MAPED_BUTTON_DOWN:
			{
				int n = activeItem+1;
				if(n >= (int)items.size())
				{
					n = 0;
				}
				SwitchToPlayer(n);
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
				items[activeItem]->callback(this);
				if(items[activeItem]->body)
				{
					items[activeItem]->body->FadeOut();
					items[activeItem]->body = 0;
				}
			}
			break;
		case MAPED_BUTTON_SC:
			win->PlayMenuCancelSound();
			SwitchToPlayer(items.size() - 1);

			items[activeItem]->callback(this);
//			on_cancel(0);
			break;
		default:
			return false;
		}
		return true;
	}

	void PlayerMenu::Init(int _activeItem)
	{
		activeItem = _activeItem;
		for(uint i=0; i<items.size(); i++)
		{
			Item * item = items[i];
			item->body = 0;
			item->state = S_INACTIVE;
			item->timer = 0;
		}
		SwitchToPlayer(activeItem);
	}
}

