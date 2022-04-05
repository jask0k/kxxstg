
#ifndef NB_COLOR_H
#define NB_COLOR_H

#include <nbug/core/def.h>

namespace e
{
	struct HSLA
	{
		float h, s, l, a;
		bool IsNormal() const
		{ return h >= 0 && h <= 1.0f && s >= 0 && s <= 1.0f && l >= 0 && l <= 1.0f && a >= 0 && a <= 1.0f; }
	};

	struct RGBA
	{
		float r, g, b, a; 
		void SetRgbaDword(uint32 _v)
		{
			r = ((_v >>  0) & 0x000000ff) * 0.0039215686f;
			g = ((_v >>  8) & 0x000000ff) * 0.0039215686f;
			b = ((_v >> 16) & 0x000000ff) * 0.0039215686f;
			a = ((_v >> 24) & 0x000000ff) * 0.0039215686f;
		}
		void SetBgraDword(uint32 _v)
		{
			b = ((_v >>  0) & 0x000000ff) * 0.0039215686f;
			g = ((_v >>  8) & 0x000000ff) * 0.0039215686f;
			r = ((_v >> 16) & 0x000000ff) * 0.0039215686f;
			a = ((_v >> 24) & 0x000000ff) * 0.0039215686f;
		}
		uint32 GetRgbaDword() const
		{
			return ((uint32)(r * 255.0f) << 0) 
				 | ((uint32)(g * 255.0f) << 8)
				 | ((uint32)(b * 255.0f) << 16) 
				 | ((uint32)(a * 255.0f) << 24);
		}
		uint32 GetBgraDword() const
		{
			return ((uint32)(b * 255.0f) << 0) 
				 | ((uint32)(g * 255.0f) << 8)
				 | ((uint32)(r * 255.0f) << 16) 
				 | ((uint32)(a * 255.0f) << 24);
		}
		void GetHsl(HSLA & _v) const;
		void SetHsl(const HSLA & _v);

		bool IsNormal() const
		{ return r >= 0 && r <= 1.0f && g >= 0 && g <= 1.0f && b >= 0 && b <= 1.0f && a >= 0 && a <= 1.0f; }

		// static RGBA GetWeb216(uint _index);
	};

	static inline RGBA MakeColor(uint8 _r, uint8 _g, uint8 _b, uint8 _a = 0xff)
	{
		RGBA ret = 
		{
			float(_r) * 0.0039215686f,
			float(_g) * 0.0039215686f,
			float(_b) * 0.0039215686f,
			float(_a) * 0.0039215686f,
		};
		return ret;
	}
}

#endif
