#include <z_kxx/boss/a/boss_a_sc1_bg.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	BossASC1BG::BossASC1BG()
	{
		tex = kxxwin->LoadTex("boss-a-sc1-bg");
		w = (float)tex->W();
		h = (float)tex->H();
		timer = S2F(2.0f);
		angle = 0;
		angle_delta = 6 * PI / timer;
		alpha = 0;
		alpha_delta = 0.7f / timer;
		state = 0;
	}

	BossASC1BG::~BossASC1BG()
	{
	}

	bool BossASC1BG::Step()
	{
		switch(state)
		{
		case 0:
			if(--timer == 0)
			{
				state = 1;
				timer = S2F(2.0f);
				angle = 0;
			}
			else
			{
				alpha+= alpha_delta;
				angle+= angle_delta;
			}
			break;
		case 1:
			if(alpha > 0.7f)
			{
				alpha = 0.7f;
				alpha_delta = -alpha / timer;
			}
			else if( alpha < 0.2f)
			{
				alpha = 0.2f;
				alpha_delta = alpha / timer;
			}
			alpha+=alpha_delta;
			break;
		case 2:
			if(--timer == 0)
			{
				return false;
			}
			else
			{
				alpha-= alpha_delta;
				angle-= angle_delta;
			}
			break;
		}
		return true;
	}

	void BossASC1BG::FadeOut()
	{
		state = 2;
		timer = S2F(1.0f);
		angle = 0;
		angle_delta = 6 * PI / timer;
		alpha = 0.7f;
		alpha_delta = alpha / timer;

	}

	void BossASC1BG::Render()
	{
		kxxwin->SetToGame2DProjection();
		graphics->PushMatrix();
		graphics->TranslateMatrix(K_GAME_XC, K_GAME_YC);
		graphics->RotateMatrix(angle, 1, 0, 0);
		graphics->SetColor(1, 1, 1, alpha);
		graphics->SetBlendMode(BM_NORMAL);
		graphics->BindTex(tex);
		graphics->DrawQuad(-K_GAME_XC, -K_GAME_YC,K_GAME_XC, K_GAME_YC);
		graphics->PopMatrix();
	}

}
