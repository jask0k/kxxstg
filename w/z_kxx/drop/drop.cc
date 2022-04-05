
// #include "../config.h"
#include <z_kxx/drop/drop.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	void Drop::Render()
	{
		if(tex1)
		{
			// graphics->BlendOn();
			graphics->SetBlendMode(BM_ADD);
			uint32 localTimeTick = kxxwin->GetRenderTimer();
			float a = localTimeTick * -0.40f;
			float w = hsz.y * 3.0f;
			graphics->PushMatrix();
			graphics->TranslateMatrix(this->pos.x, this->pos.y, 0);
			graphics->RotateMatrix(a, 0, 0, 1);
			graphics->BindTex(tex1);
			graphics->DrawQuad(-w, -w, w, w);
			graphics->PopMatrix();
			graphics->SetBlendMode(BM_NORMAL);
		}
		Sprite::Render();
	}

	bool Drop::Step()
	{
		static const float DROP_ABSORB_SPEED = PS2PF(1300);

		Player * player = kxxwin->player;
		int n = player->TestAbsorbDrop(pos, this->hsz.x);
		if(!absorbing && absorb_mode != NO_ABSORB &&
			(n == 1 
			|| player->pos.y < K_GAME_H * K_ITEM_GET_LINE 
			|| absorb_mode == AUTO_ABSORB) )
		{
			absorbing = true;
		}

		if(speed.y >= max_vel)
		{
			if(absorbing && player->pos.y < K_GAME_H)
			{
				// approach to player on absorb state
				if(absorbSpeed < DROP_ABSORB_SPEED)
				{
					absorbSpeed+= 0.04f;
				}
				pos+= (player->pos - pos).GetNormal() * absorbSpeed;
			}
			pos.y+= max_vel;
		}
		else
		{
			// on initial track
			pos+= speed;
			ang+= angle_delta;
			speed.y+= 1.75f / K_LOGIC_FPS;

			if(speed.y >= max_vel)
			{
				ang = 0;
			}
		}

		if(n == 2)
		{
			// absorbed by player
			player->OnAteDrop(dropType);
			return false;
		}
		return pos.y < K_GAME_H + 50;
	}

}
