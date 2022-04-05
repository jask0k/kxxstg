
// #include "../config.h"
#include <z_kxx/ui/main_menu.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/sprite/dandelions.h>
#include <z_kxx/sprite/static_pic.h>
#include <z_kxx/sprite/dynamic_pic.h>

namespace e
{
#define MAX_FADE_IN 60
	MainMenu::MainMenu(KxxWin * _win)
		: UIBase(_win)
	{
		activeItem = 0;
		fontFace = 0;
		disabledFontColor = MakeColor(64, 64, 64);
		defaultFontColor = MakeColor(160, 160, 64);
		bg = enew StaticImage("menu-bg", 0, 0, K_VIEW_W, K_VIEW_H);
		bg->blm = BM_DISABLE;
		bg->AddToRenderList(RL_UI_BACK_0);
		bg->clr.r = 1;
		bg->clr.g = 1;
		bg->clr.b = 1;
		bg->clr.a = 0;

		dandelions = enew Dandelions();
		kxxwin->AddUISpriteToList(dandelions, RL_UI_BACK_1);
		TexRef titleTex = kxxwin->LoadTex("main-title");
		if(titleTex)
		{
			title = enew DynamicImage("main-title", K_VIEW_W - 20 - (float)titleTex->W(), 20, K_VIEW_W - 20, 20 + (float)titleTex->H());
			title->method = DynamicImage::FlipH;
			title->SetTimer(30, 0, 30, false);
			kxxwin->AddUISpriteToList(title, RL_UI_FRONT_2);
		}
		else
		{
			title = 0;
		}
		bodyTex = 0;//kxxwin->LoadTex("player_a_0");
		fadeInTimer = MAX_FADE_IN;
		joystickIdleTimer = 0;

		btn1Tex = kxxwin->LoadTex("btn1");
		btn2Tex = kxxwin->LoadTex("btn2");
	}

	MainMenu::~MainMenu()
	{
		if(title)
		{
			title->FadeOut();
		}
		//if(bodyTex)
		//{
		//	bodyTex->Release();
		//}
		dandelions->BlowAway();
		delete bg;
		for(uint i=0; i<items.size(); i++)
		{
			delete items[i];
		}
	}

	void MainMenu::Step()
	{
		joystickIdleTimer++;
		if(joystickIdleTimer >= 6 * 60)
		{
			joystickIdleTimer = 0;
			if(kxxwin->StartDemoReplay())
			{
				return;
			}
		}
		else if(joystickIdleTimer == 5 * 60)
		{
			kxxwin->ShowNotifyMessage(_TT("KEYS: <Arrow keys> Z X <Left Shift>"), 4);
		}
	}

	void MainMenu::RenderBack()
	{
	}

	void MainMenu::RenderFront()
	{
		// NB_PROFILE_INCLUDE;
		graphics->SetColor(1, 1, 1, 1);
		graphics->SetBlendMode(BM_NORMAL);
		if(fadeInTimer)
		{
			fadeInTimer--;
		}

		float fade = 1.0f - float(fadeInTimer) / MAX_FADE_IN;
		// title
		// graphics->SetTexMode(TextureMode::replace);
		// graphics->BlendOn();
		// graphics->SetTexMode(TM_MODULATE);


		// menu color block
		//graphics->SetTexMode(TM_DISABLE);
		//graphics->SetColor(0, 0, 0, 0.5f);
//		graphics->DrawQuad(menu_x0 - 10, menu_y0 - 10, menu_x0 + menu_w + 10, menu_y0 + menu_h + 10);

		// draw menu item
		//float x0 = menu_x0;
		//float x1 = x0 + menu_w;
		//float y0 = menu_y0;
		float aa = 0;
		for(uint i=0; i<items.size(); i++)
		{
			Item * item = items[i];
			switch(item->step)
			{
			case 0:
				if(item->timer)
				{
					item->timer--;
					if(item->timer == 0)
					{
						item->step=1;
						item->speed = 0;
					}
				}
				break;
			case 1:
				item->speed+= 0.08f;
				item->rect.x+= item->speed;
				if(item->rect.x >= -item_w * 0.1f)
				{
					item->step = 2;
				}
				break;
			case 2: 
				item->rect.x-= 0.2f;
				if(item->rect.x <= -item_w * 0.2f)
				{
					item->step = 3;
				}
				break;
			default:
				break;
			}
			float x0 = item->rect.x;
			float y0 = item->rect.y;
			float x1 = item->rect.R();
			float y1 = item->rect.B();
			graphics->SetTexMode(TM_MODULATE);
			float a = 1 - (-x0 / item_w);
		//	E_TRACE_LINE(string(a));
			if(a < 0 )
			{
				a = 0;
			}
			if(a > 1)
			{
				a = 1;
			}
			if(i == 1)
			{
				aa = a;
			}
			graphics->SetBlendMode(BM_ADD);
			graphics->SetColor(1, 1, 1, a);
			if(this->activeItem == i)
			{
				graphics->BindTex(btn2Tex);
			}
			else
			{
				graphics->BindTex(btn1Tex);
			}
			graphics->DrawQuad(x0, y0, x1, y1);
			graphics->SetBlendMode(BM_NORMAL);
			//graphics->SetColor(item->enabled? defaultFontColor : disabledFontColor);
			//graphics->SetColor(1, 1, 1, 1);
			if(item->enabled)
			{
				graphics->SetColor(1, 1, 1, 1);
			}
			else
			{
				graphics->SetColor(0.5, 0.5, 0.5, 1);
			}
			graphics->SetFont(fontFace); 
			graphics->DrawString(x0 + item_w - text_w - 10, y0 + (item_h - text_h) * 0.5f, item->text.c_str(), item->text.length());
		}
		float mm = aa;
		mm = mm * 0.4f;
		if(mm > 0.2f)
		{
			mm = 0.2f;
		}
		mm = 1.0f - mm;

		float gg = (1.0f - aa*0.7f);
		{
			//  0, 0, K_VIEW_W, K_VIEW_H float
			bg->bound.x = - mm * K_VIEW_W * 0.2f;
			float r = K_VIEW_W * (1.0f + mm * 0.2f);
			bg->bound.w = r - bg->bound.x;

			bg->bound.y = - mm * K_VIEW_H * 0.2f;
			float b = K_VIEW_H * (1.0f + mm * 0.2f);
			bg->bound.h = b - bg->bound.y;
			bg->clr.a = gg;
		}
	}

	void MainMenu::CalcMenuSize()
	{
//		margin = 0.2f;
		text_w = 0;
		float y0 = 150;
		float & w = item_w;
		float & h = item_h;
		if(btn1Tex)
		{
			w = (float) btn1Tex->W();
			h = (float) btn1Tex->H();
		}
		else
		{
			w = 100;
			h = 30;
		}
		for(uint i=0; i<items.size(); i++)
		{
			Item * item = items[i];
			item->rect.x = -w;
			item->rect.y = y0 + i * (h + 10);
			item->rect.w = w;
			item->rect.h = h;
			item->step = 0;
			item->timer = i * 10 + 1;
			float w1 = (float) fontFace->W(item->text.c_str(), item->text.length());
			if(w1 >text_w)
			{
				text_w = w1;
			}
		}
		text_h = (float) fontFace->H(); //
	}


	void MainMenu::SetBoundRect(float _x0, float _y0, float _x1, float _y1) 
	{
		UIBase::SetBoundRect(_x0, _y0, _x1, _y1);
		CalcMenuSize();
	}

	bool MainMenu::OnJoystickDown(int _joystick, int _button)
	{
		E_ASSERT(win != 0);
		if(_joystick != 0)
		{
			return false;
		}
		joystickIdleTimer = 0;
		switch(_button)
		{
		case MAPED_BUTTON_UP:
			//if(keyControlTipTimer > 0 && keyControlTipTimer < 5 * 60)
			//{
			//	keyControlTipTimer = 5 * 60;
			//}
			activeItem--;
			if(activeItem < 0)
			{
				activeItem = items.size() - 1;
			}
			kxxwin->lastMainMenuItem = activeItem;
			win->PlayMenuSelectSound();
			break;
		case MAPED_BUTTON_DOWN:
			//if(keyControlTipTimer > 0 && keyControlTipTimer < 5 * 60)
			//{
			//	keyControlTipTimer = 5 * 60;
			//}
			activeItem++;
			if(activeItem >= (int)items.size())
			{
				activeItem = 0;
			}
			win->PlayMenuSelectSound();
			kxxwin->lastMainMenuItem = activeItem;
			break;
		case MAPED_BUTTON_FIRE:
			if(items[activeItem]->enabled)
			{
				items[activeItem]->callback(0);
				win->PlayMenuActionSound();
			}
			break;
		default:
			return false;
		}
		return true;
	}

}

