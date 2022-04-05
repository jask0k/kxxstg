
// #include "../config.h"
#include <z_kxx/player/player_b_shot1.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
#define TOTAL_TIME S2F(0.4f)
#define IN_TIME S2F(0.2f)
#define OUT_TIME S2F(0.2f)
	PlayerBShot1::PlayerBShot1(const Vector2 & _pt, TexRef _tex, TexRef _petTex, float _power)
	{
		fragile = false;
		power = _power;

		tex = _tex;
		petTex = _petTex;
		timer = TOTAL_TIME;
		hsz.x = 0;
		pos = _pt;
		y0 = y1 = y2 = _pt.y;
		hue = 2;
	}


	PlayerBShot1::~PlayerBShot1()
	{
	}

	void PlayerBShot1::Render()
	{
		graphics->SetBlendMode(BM_ADD);
		graphics->BindTex(tex);
		graphics->SetColor(0, 1, 1, 0.5f);
		graphics->DrawQuad(pos.x - hsz.x * 2.0f, y2,  pos.x + hsz.x * 2.0f, y1);
		graphics->SetColor(1, 0.5f, 0.5f, 1);
		graphics->DrawQuad(pos.x - hsz.x * 1.0f, y0,  pos.x + hsz.x * 1.0f, y1);
		graphics->SetColor(0.5f, 0.5f, 1, 1);
		graphics->DrawQuad(pos.x - hsz.x * 0.5f, y0,  pos.x + hsz.x * 0.5f, y1);
		graphics->SetBlendMode(BM_NORMAL);

		//Vector2 & v = pos;
		//graphics->PushMatrix();
		//graphics->TranslateMatrix(v.x, v.y, 0);
		//float a = kxxwin->GetRenderTimer() * 0.05f;
		//graphics->RotateMatrix(a, 0, 0, 1);

		//if(petTex)
		//{
		//	graphics->BindTex(petTex);
		//	if(clr.a < 0.99f)
		//	{
		//		// graphics->SetTexMode(TextureMode::Modulate);
		//		graphics->SetColor(1, 1, 1, clr.a);
		//		graphics->DrawQuad(-10, -10, 10, 10);
		//		// graphics->SetTexMode(TextureMode::replace);
		//	}
		//	else
		//	{
		//		graphics->DrawQuad(-10, -10, 10, 10);
		//	}
		//}

		//graphics->PopMatrix();

	}

	bool PlayerBShot1::Step()
	{
		if(timer)
		{
			timer--;

			if(timer < (uint)OUT_TIME)
			{
				float a1 = float(timer) / OUT_TIME;
				hsz.x = 10 * a1;
			}
			else if(timer > (uint)(TOTAL_TIME - IN_TIME))
			{
				float a1 = 1.0f - (float(timer) - (TOTAL_TIME - IN_TIME)) / IN_TIME;
				hsz.x = 10 * a1;
			}
			float f = 1.0f - (float)timer/TOTAL_TIME;
			y0 =  pos.y - K_GAME_H * 3.0f * f;
			y2 =  pos.y - K_GAME_H * 1.5f * f;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool PlayerBShot1::Collide(const Vector2 & _v, float _r)
	{
		return timer == TOTAL_TIME - IN_TIME  
			&& _v.x - _r <= pos.x + hsz.x && _v.x + _r >= pos.x - hsz.x
			&& _v.y + _r >= y0 && _v.y - _r <= y1;
	}
}

