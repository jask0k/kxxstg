
// #include "../config.h"
#include <z_kxx/shot/petal_shot.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	PetalShot::PetalShot(float _x, float _y, float _a)
	{
		timer1 = (int)(0.3f * K_LOGIC_FPS);
		E_ASSERT(timer1 > 0);
		tex1 = kxxwin->shotSmokeTex0;
		tex2 = kxxwin->shotTex[0][0];

		r = 20;
		dr = (10.0f - 20.0f) / timer1;
		hsz.x = r;
		hsz.y = r;

		timer2 = (int)(1.0f * K_LOGIC_FPS);
		E_ASSERT(timer2 > 0);
		float speed0_f = shot_vel(10);
		float speed1_f = shot_vel(5);
		float sin_a = sin(_a);
		float cos_a = cos(_a);
		float accel_f = (speed1_f - speed0_f) / timer2;
		accel.x = accel_f * cos_a;
		accel.y = accel_f * sin_a;
		speed.x = speed0_f * cos_a;
		speed.y = speed0_f * sin_a;
		pos.x = _x + speed.x;
		pos.y = _y + speed.y;
		RotateNorthToVectorDirection(speed);
		hue = 3;
	}

	PetalShot::~PetalShot()
	{
	}

	bool PetalShot::Step()
	{
		if(timer1)
		{
			timer1--;
			r+= dr;
			if(timer1)
			{
				hsz.x = r;
				hsz.y = r;
			}
			else
			{
				hsz.x = tex2->W() * 0.5f;
				hsz.y = tex2->H() * 0.5f;
			}
			return true;
		}
		else 
		{
			pos.x+= speed.x;
			pos.y+= speed.y;
			if(timer2)
			{
				timer2--;
				speed.x+= accel.x;
				speed.y+= accel.y;
			}
			return !DisappearTest();
		}
	}

	void PetalShot::Render() 
	{
		if(timer1)
		{
			tex = tex1;
			Sprite::Render();
		}
		else
		{
			tex = tex2;
			Sprite::Render();
		}
	}

	bool PetalShot::Collide(const Vector2 & _v, float _r)
	{
		if(timer1)
		{
			return false;
		}
		else
		{
			float a = -PI * 0.5f - ang;
			float b = hsz.y * 0.5f;
			float x0 = b * cos(a);
			float y0 = b * sin(a);
			float dx = _v.x - x0 - pos.x;
			float dy = _v.y - y0 - pos.y;
			_r = _r + 3;
			return dx*dx + dy*dy <= _r * _r;
		}
	}

	void PetalShot::RenderDebug()
	{
#ifdef NB_DEBUG
		if(timer1 == 0)
		{
			RGBA color = kxxwin->debugFontColor;
			float a = -PI * 0.5f + ang;
			float b = hsz.y * 0.5f;
			float x0 = b * cos(a);
			float y0 = b * sin(a);
			graphics->DrawEllipse(pos.x+x0, pos.y+y0, ang, 3, 3, color, false);
		}
#endif
	}

}
