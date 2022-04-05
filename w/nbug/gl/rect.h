
#pragma once

#include <nbug/math/math.h>

namespace e
{
	struct Rect
	{
		//union
		//{
		//	struct
		//	{
				float x, y, w, h;
		//	};
		//	Vector2 pos;
		//	Vector2 size;
		//};

		float L() const
		{ return x; }
		float T() const
		{ return y; }
		float R() const
		{ return x + w; }
		float B() const
		{ return y + h; }
		//float X() const
		//{ return x; }
		//float Y() const
		//{ return y; }
		//float W() const
		//{ return w; }
		//float H() const
		//{ return h; }
		float Area() const
		{ return w * h; }
		void Normalize()
		{
			if(w < 0)
			{
				x+= w;
				w = -w;
			}
			if(h < 0)
			{
				y+= h;
				h = -h;
			}
		}

		void operator&=(const Rect & _r)
		{
			float r1 = x + w;
			float r2 = _r.x + _r.w;
			float b1 = y + h;
			float b2 = _r.y + _r.h;

			float l, t, r, b;
			l = x < _r.x ? _r.x : x;
			t = y < _r.y ? _r.y : y;
			r = r1 < r2 ? r1 : r2;
			b = b1 < b2 ? b1 : b2;
			x = l;
			y = t;
			if(l < r && t < b)
			{
				w = r - l;
				h = b - t;
			}
			else
			{
				w = 0;
				h = 0;
			}
		}

		Rect operator&(const Rect & _r) const
		{
			Rect ret = *this;
			ret&=(_r);
			return ret;
		}

		void operator|=(const Rect & _r)
		{
			float r1 = x + w;
			float r2 = _r.x + _r.w;
			float b1 = y + h;
			float b2 = _r.y + _r.h;

			float l, t, r, b;
			l = x > _r.x ? _r.x : x;
			t = y > _r.y ? _r.y : y;
			r = r1 > r2 ? r1 : r2;
			b = b1 > b2 ? b1 : b2;
			x = l;
			y = t;
			w = r - l;
			h = b - t;
		}

		Rect operator|(const Rect & _r) const
		{
			Rect ret = *this;
			ret|=(_r);
			return ret;
		}

		bool Contains(float _x, float _y) const
		{ 		
			//E_ASSERT(w>=0 && h>=0);
			return _x >= x && _y >= y && _x <= x+w && _y <= y+h;
		 }
	};

	//struct Recti
	//{
	//	int x, y;
	//	uint w, h;

	//	int L() const
	//	{ return x; }
	//	int T() const
	//	{ return y; }
	//	int R() const
	//	{ return x + w - 1; }
	//	int B() const
	//	{ return y + h - 1; }
	//	int Area() const
	//	{ return w * h; }

	//	void operator&=(const Recti & _r)
	//	{
	//		int r1 = R();
	//		int r2 = _r.R();
	//		int b1 = B();
	//		int b2 = _r.B();

	//		int r, b;
	//		x = x < _r.x ? _r.x : x;
	//		y = y < _r.y ? _r.y : y;
	//		r = r1 < r2 ? r1 : r2;
	//		b = b1 < b2 ? b1 : b2;
	//		if(x < r && y < b)
	//		{
	//			w = r - x + 1;
	//			h = b - y + 1;
	//		}
	//		else
	//		{
	//			w = 0;
	//			h = 0;
	//		}
	//	}

	//	Recti operator&(const Recti & _r) const
	//	{
	//		Recti ret = *this;
	//		ret&=(_r);
	//		return ret;
	//	}

	//	void operator|=(const Recti & _r)
	//	{
	//		int r1 = R();
	//		int r2 = _r.R();
	//		int b1 = B();
	//		int b2 = _r.B();

	//		int r, b;
	//		x = x > _r.x ? _r.x : x;
	//		y = y > _r.y ? _r.y : y;
	//		r = r1 > r2 ? r1 : r2;
	//		b = b1 > b2 ? b1 : b2;
	//		w = r - x + 1;
	//		h = b - y + 1;
	//	}

	//	Recti operator|(const Recti & _r) const
	//	{
	//		Recti ret = *this;
	//		ret|=(_r);
	//		return ret;
	//	}

	//	bool Contains(int _x, int _y) const
	//	{ return _x >= x && _y >= y && _x <= R() && _y <= B(); }
	//};
}
