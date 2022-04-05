
// #include "../config.h"
#include <z_kxx/shot/laser.h>

namespace e
{
	Laser::Laser(Sprite * _master, TexRef _tex, float _w, float _a, float _da, float _r0, float _r, uint32 _t)
	{
		tex = _tex;
		r0 = _r0;
		fragile = false;
		angle_delta = _da;
		scl.x = 1.0f;
		scl.y = 0.0f;
		hsz.x = _r*0.5f;
		hsz.y = _w * 0.5f;
		float sin_a = sin(_a);
		float cos_a = cos(_a);
		timer_state2 = _t;
		state = 0;
		isEllipseCollision = false;
		collisionFrac.x = 1.0f;
		_master->AddPet(this);
		SetAndle(_a);
		timer = 120;
	}

	void Laser::SetAndle(float _a)
	{
		ang = _a;
		float x = (r0 + hsz.x) * cos(_a);
		float y = (r0 + hsz.x) * sin(_a);
		pos.x = master->pos.x + x;
		pos.y = master->pos.y + y;
	}

	bool Laser::Step()
	{
		if(!master)
		{
			return false;
		}
		
		switch(state)
		{
		case 0:
			if(--timer == 0)
			{
				state = 1;
				timer = 120;
				scale_delta = 1.0f / timer;
			}
			break;
		case 1:
			if(--timer == 0)
			{
				state = 2;
				timer = timer_state2;
				scl.y = 1.0f;

			}
			else
			{
				ang+=angle_delta;
				scl.y+= scale_delta;
			}
			break;
		case 2:
			if(--timer == 0)
			{
				state = 3;
				timer = 120;
				scale_delta = -(0.99f / timer);
			}
			else
			{
				ang+=angle_delta;
			}
			break;
		case 3:
			if(--timer == 0)
			{
				return false;
			}
			else
			{
				ang+=angle_delta;
				scl.y+= scale_delta;
			}
			break;
		}
		SetAndle(ang);
		return true;
	}

	void Laser::Render()
	{
		if(state)
		{
			EnemyShot::Render();
		}
		else
		{
			graphics->SetColor(clr);
			graphics->SetBlendMode(blm);

			graphics->PushMatrix();
			graphics->TranslateMatrix(pos.x, pos.y, 0);
			if(ang)
			{
				graphics->RotateMatrix(ang, 0, 0, 1);
			}
			float scaledHalfWidth = hsz.x * scl.x;
			float scaledHalfHeight = hsz.y * scl.y;
			//graphics->BindTex(tex);
			//graphics->DrawQuad(-scaledHalfWidth, -scaledHalfHeight, scaledHalfWidth, scaledHalfHeight);
			graphics->DrawLine(-scaledHalfWidth, 0, 0, clr, scaledHalfWidth, 0, 0, clr);
			graphics->PopMatrix();
		}
	}

	bool Laser::Collide(const Vector2 & _v, float _r)
	{
		return state ? EnemyShot::Collide(_v, _r) : false;
	}

	void Laser::OnHit(const Vector2 & _pt)
	{
	}

	void Laser::OnCrash(const Vector2 & _pt)
	{
	}

	void Laser::DropItem()
	{
		if(state)
		{
		}
	}

}
