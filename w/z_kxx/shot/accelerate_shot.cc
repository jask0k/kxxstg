
// #include "../config.h"
#include <z_kxx/shot/accelerate_shot.h>

namespace e
{
	AccelerateShot::AccelerateShot(TexRef _tex, const Vector2 & _speed0, const Vector2 & _speed1, float _second)
	{
		timer1 = 0;
		tex1 = 0;
		tex2 = _tex;
		tex = _tex;
		if(tex2)
		{
			//tex2->AddRef();
			hsz.x  = tex2->W() * 0.5f;
			hsz.y = tex2->H() * 0.5f;
		}
		timer2 = S2F(_second);
		E_ASSERT(timer2 > 0);
		accel.x = (_speed1.x - _speed0.x) / timer2;
		accel.y = (_speed1.y - _speed0.y) / timer2;
		speed = _speed0;
		RotateNorthToVectorDirection(speed);
	}

	AccelerateShot::AccelerateShot(TexRef _tex, const Vector2 & _speed0, const Vector2 & _speed1, float _second
			, TexRef _smokeTex, float _r0, float _r1, float _second1)
	{
		timer1 = S2F(_second1);
		E_ASSERT(timer1 > 0);
		tex1 = _smokeTex;
		//if(tex1)
		//{
		//	tex1->AddRef();
		//}
		r = _r0;
		dr = (_r1 - _r0) / timer1;
		hsz.x = r;
		hsz.y = r;

		tex = _tex;
		tex2 = _tex;
		//if(tex2)
		//{
		//	tex2->AddRef();
		//}
		timer2 = S2F(_second);
		E_ASSERT(timer2 > 0);
		accel.x = (_speed1.x - _speed0.x) / timer2;
		accel.y = (_speed1.y - _speed0.y) / timer2;
		speed = _speed0;
	//	RotateNorthToVectorDirection(speed);
	}

	AccelerateShot::~AccelerateShot()
	{
		//if(tex1)
		//{
		//	tex1->Release();
		//}
		//if(tex2)
		//{
		//	tex2->Release();
		//}
	}

	bool AccelerateShot::Step()
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
				//RotateNorthToVectorDirection(speed);
			}
			return !DisappearTest();
		}
	}

	void AccelerateShot::Render() 
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

	bool AccelerateShot::Collide(const Vector2 & _v, float _r)
	{
		return timer1 ? false : EnemyShot::Collide(_v, _r);
	}
}
