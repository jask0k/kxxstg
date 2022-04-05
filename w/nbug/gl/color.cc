
#include "private.h"
#include <nbug/gl/color.h>
#include <nbug/core/debug.h>

namespace e
{

	void RGBA::GetHsl(HSLA & _v) const
	{
		// E_PROFILE_INCLUDE
		E_ASSERT(this->IsNormal());
		_v.a = this->a;

		float r1, g1, b1,h, s, l;
		r1 = this->r;
		g1 = this->g;
		b1 = this->b;

		float max1 = r1;
		max1 = E_MAX(max1, g1);
		max1 = E_MAX(max1, b1);
		float min1 = r1;
		min1 = E_MIN(min1, g1);
		min1 = E_MIN(min1, b1);


		if(max1 == min1)
		{
			_v.h = 0.627f;
			_v.s = 0;
			_v.l = this->r;
			return;
		}

		float max_sub_min = max1 - min1;
		float max_add_min = max1 + min1;
		l = max_add_min / 2.0f;

		if(l < 0.5)
		{
			s = max_sub_min / max_add_min;
		}
		else
		{
			s = max_sub_min / (2.0f - max_add_min);
		}

		if(r1 == max1)
		{
			h = (g1 - b1) / max_sub_min;
		}
		else if(g1 == max1)
		{
			h = 2.0f + (b1 - r1) / max_sub_min;
		}
		else /*if(b1 == max1)*/
		{
			h = 4.0f + (r1 - g1) / max_sub_min;
		}

		if(h<0) h+= 6.0f;
		_v.h = h  / 6.0f;
		_v.s = s;
		_v.l = l;
	}

	void RGBA::SetHsl(const HSLA & _v)
	{
		// E_PROFILE_INCLUDE
		E_ASSERT(_v.IsNormal());
		this->a = _v.a;

		if(_v.s == 0)
		{
			this->r = _v.l;
			this->g = _v.l;
			this->b = _v.l;
			return;
		}
		float h, s, l;
		h = _v.h;
		s = _v.s;
		l = _v.l;

		float t2;
		if(l < 0.5)
		{
			t2 = l * (1.0f + s);
		}
		else
		{
			t2 = l + s - l * s;
		}

		float t1 = 2.0f * l - t2;
		float t3_r = h + 1.0f/3.0f;
		float t3_g = h;
		float t3_b = h - 1.0f/3.0f;
		if(t3_r > 1.0f) t3_r-= 1.0f;
		if(t3_b < 0.0f) t3_b+= 1.0f;
		E_ASSERT(t3_r>=0.0f && t3_r<=1.0f);
		E_ASSERT(t3_g>=0.0f && t3_g<=1.0f);
		E_ASSERT(t3_b>=0.0f && t3_b<=1.0f);

		float r1, g1, b1;
		if(6.0f * t3_r < 1.0f)
		{
			r1 = t1 + (t2 - t1) * 6.0f * t3_r;
		}
		else if(2.0f * t3_r < 1.0f)
		{
			r1 = t2;
		}
		else if(3.0f * t3_r < 2.0f)
		{
			r1 = t1 + (t2 - t1) * 6.0f * ((2.0f/3.0f)-t3_r);
		}
		else
		{
			r1 = t1;
		}

		if(6.0f * t3_g < 1.0f)
		{
			g1 = t1 + (t2 - t1) * 6.0f * t3_g;
		}
		else if(2.0f * t3_g < 1.0f)
		{
			g1 = t2;
		}
		else if(3.0f * t3_g < 2.0f)
		{
			g1 = t1 + (t2 - t1) * 6.0f * ((2.0f/3.0f)-t3_g);
		}
		else
		{
			g1 = t1;
		}

		if(6.0f * t3_b < 1.0f)
		{
			b1 = t1 + (t2 - t1) * 6.0f * t3_b;
		}
		else if(2.0f * t3_b < 1.0f)
		{
			b1 = t2;
		}
		else if(3.0f * t3_b < 2.0f)
		{
			b1 = t1 + (t2 - t1) * 6.0f * ((2.0f/3.0f)-t3_b);
		}
		else
		{
			b1 = t1;
		}

		//int r2 = int(r1 * 256.0f); if(r2 >= 256) r2 = 255;
		//int g2 = int(g1 * 256.0f); if(g2 >= 256) g2 = 255;
	//	int b2 = int(b1 * 256.0f); if(b2 >= 256) b2 = 255;

		this->r = r1;
		this->g = g1;
		this->b = b1;
	}

	/*
	RGBA RGBA::GetWeb216(uint _index)
	{
		if(_index >= 216)
		{
			_index = _index % 216;
		}
		_index = 215 - _index;
		int r = _index % 6;
		int g = _index / 6 % 6;
		int b = _index / 36;
		const float d = 1.0f / 5.0f;
		RGBA ret = {d*r, d*g, d*b, 1};
		return ret;
	}
	 */
}

