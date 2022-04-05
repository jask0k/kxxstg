
// #include "../config.h"
#include <z_kxx/player/player_b_sc.h>
#include <z_kxx/player/player_b.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	MasterSpark::MasterSpark(PlayerB * _player)
	{
		fragile = false;
		power = 14.0f;
		player = _player;
		tex = _player->texMasterSpark;
		timer = SPELL_CARD_DURATION;
		pos = _player->pos;
		da = 0;
		w  = 0;
		hue = 2;
	}


	MasterSpark::~MasterSpark()
	{
	}

	void MasterSpark::Render()
	{
		graphics->SetBlendMode(BM_ADD);
		graphics->BindTex(tex);
		graphics->PushMatrix();
		graphics->TranslateMatrix(pos.x, pos.y, 0);


		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->RotateMatrix(-PI*0.5f - 2.5f*da, 0, 0, 1);
		graphics->SetColor(0.3f, 0.3f, 0.9f, 0.3f);
		graphics->DrawQuad(0, -w, 700, w);
		graphics->RotateMatrix(da, 0, 0, 1);
		graphics->SetColor(  0.5f, 0.5f, 0.9f, 0.4f);
		graphics->DrawQuad(0, -w, 700, w);
		graphics->RotateMatrix(da, 0, 0, 1);
		graphics->SetColor( 0.7f, 0.7f, 0.9f, 0.5f);
		graphics->DrawQuad(0, -w, 700, w);
		graphics->RotateMatrix(da, 0, 0, 1);
		graphics->SetColor( 0.9f, 0.7f, 0.7f, 0.5f);
		graphics->DrawQuad(0, -w, 700, w);
		graphics->RotateMatrix(da, 0, 0, 1);
		graphics->SetColor(  0.9f, 0.5f, 0.5f, 0.4f);
		graphics->DrawQuad(0, -w, 700, w);
		graphics->RotateMatrix(da, 0, 0, 1);
		graphics->SetColor( 0.9f, 0.3f, 0.3f, 0.3f);
		graphics->DrawQuad(0, -w, 700, w);
		// graphics->SetTexMode(TextureMode::replace);

		graphics->PopMatrix();
		graphics->SetBlendMode(BM_NORMAL);
	}

	bool MasterSpark::Step()
	{
		pos = player->pos;
		if(timer)
		{
			unavailableTimer++;
			if(unavailableTimer > K_LOGIC_FPS_MUL * 12)
			{
				unavailableTimer = 0;
			}
		
			timer--;
			float split0 = SPELL_CARD_DURATION * 0.3f;
			float split = SPELL_CARD_DURATION * 0.9f;

			if(timer <= split0)
			{
				da = 0.05f * (float)timer / split0;
				w = 150 * (float)timer / split0;
			}
			else if(timer > split)
			{
				da = -0.05f * ((float)timer - SPELL_CARD_DURATION) / (SPELL_CARD_DURATION * 0.1f);
				w  = -150 * ((float)timer - SPELL_CARD_DURATION) / (SPELL_CARD_DURATION * 0.1f);
			}
			else
			{
				da = 0.05f;
				w  = 150;
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	bool MasterSpark::Collide(const Vector2 & _v, float _r)
	{
		return _v.x - _r <= pos.x + w && _v.x + _r >= pos.x - w
			&& _v.y + _r <= pos.y;
	}
}
