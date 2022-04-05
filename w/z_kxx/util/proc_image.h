
#pragma once

#include <nbug/core/def.h>
#include <nbug/math/math.h>
#include <nbug/gl/color.h>
#include <nbug/gl/tex.h>


namespace e
{
	struct RGBA;

	class ProcImageStar : public TexLoader
	{
		uint w, h;
	public:
		ProcImageStar(uint _w, uint _h)
		{
			w = _w;
			h = _h;
		}
		bool Load(Image & _pic) override
		{
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;
			for(int i=0; i<(int)_pic.w; i++)
			{
				for(int j=0; j<(int)_pic.h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
					float x = (hw - i) / hw;
					float y = (hh - j) / hh;
					x = 1.0f - fabs(x);
					y = 1.0f - fabs(y);
					if(x<0) x = 0;
					if(y<0) y = 0;
					p[3] = (int)(x*x*y*y* 255);
				}
			}
			return true;
		}
	};

	class ProcImageGradientDisc : public TexLoader
	{
		uint w, h;
		RGBA inner;
		RGBA outer;
		bool wrap;
		float exp;
	public:
		ProcImageGradientDisc(uint _w, uint _h, const RGBA & _inner, const RGBA & _outer, bool _wrap = false, float _exp = 1.0f)
		{
			w = _w;
			h = _h;
			inner = _inner;
			outer = _outer;
			wrap = _wrap;
			exp = _exp;
		}

		bool Load(Image & _pic) override
		{
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;

			for(int i=0; i<(int)w; i++)
			{
				for(int j=0; j<(int)h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					float x = (i - hw) / hw;
					float y = (j - hh) / hh;
					float f = sqrt(x*x + y*y);
					if(f > 1.0f)
					{
						if(wrap)
						{
							if(f > 0.95f)
							{
								f = (1.0f - f) * 20.0f;
								if(f < 0)
								{
									f = 0;
								}
							}
						}
						else
						{
							f = 1.0f;
						}
					}
					f = pow(f, exp);
					float r = inner.r + (outer.r - inner.r) * f;
					float g = inner.g + (outer.g - inner.g) * f;
					float b = inner.b + (outer.b - inner.b) * f;
					float a = inner.a + (outer.a - inner.a) * f;
					p[0] = (int)(r * 255);
					p[1] = (int)(g * 255);
					p[2] = (int)(b * 255);
					p[3] = (int)(a * 255);
				}
			}
			return true;
		}
	};

	class ProcImageBlastWave : public TexLoader
	{
		uint w, h;
	public:
		ProcImageBlastWave(uint _w, uint _h)
		{
			w = _w;
			h = _h;
		}

		bool Load(Image & _pic) override
		{
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;

			float r1 = 0.7f;
			float r2 = 0.8f;
			float r3 = 0.9f;
			for(int i=0; i<(int)w; i++)
			{
				for(int j=0; j<(int)h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
					float x = (i - hw) / hw;
					float y = (j - hh) / hh;
					float r = sqrt(x*x + y*y);
					float dr1 = 0.1f - fabs(r - r1);
					float dr2 = 0.1f - fabs(r - r2);
					float dr3 = 0.1f - fabs(r - r3);
					float a = E_MAX(dr1, dr2);
					a = E_MAX(a, dr3);
					a =  a * 5;
					if(a<0)
					{
						a = 0;
					}
					if(a>1.0f)
					{
						a= 1.0f;
					}
					//p[0]=p[1]=p[2]=p[3] = int(a * 255);
					p[3] = int(a * 255);
				}
			}
			return true;
		}
	};

	class ProcImageLaser : public TexLoader
	{
		uint w, h;
		RGBA color;
		bool vertical;
	public:
		ProcImageLaser(uint _w, uint _h, const RGBA & _color, bool _vertical)
		{
			w = _w;
			h = _h;
			color = _color;
			vertical = _vertical;
		}

		bool Load(Image & _pic) override
		{
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;

			for(int i=0; i<(int)w; i++)
			{
				for(int j=0; j<(int)h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					float f;
					if(vertical)
					{
						float x = (i - hw) / hw;
						f = x * x * x;
					}
					else
					{
						float y = (j - hh) / hh;
						f = y * y * y;
					}
					f = fabs(f);
					float r = 1.0f + (color.r - 1.0f) * f;
					float g = 1.0f + (color.g - 1.0f) * f;
					float b = 1.0f + (color.b - 1.0f) * f;
					float a = 1.0f + (color.a - 1.0f) * f;
					p[0] = (int)(r * 255);
					p[1] = (int)(g * 255);
					p[2] = (int)(b * 255);
					p[3] = (int)(a * 255);
				}
			}
			return true;
		}
	};

	class ProcImageExplosion : public TexLoader
	{
		uint w, h;
		RGBA outer;
	public:
		ProcImageExplosion(uint _w, uint _h, const RGBA & _outer)
		{
			w = _w;
			h = _h;
			outer = _outer;
		}

		bool Load(Image & _pic) override
		{
			RGBA inner = outer;
			inner.a = 0;
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;

			for(int i=0; i<(int)w; i++)
			{
				for(int j=0; j<(int)h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					float x = (i - hw) / hw;
					float y = (j - hh) / hh;
					float f = sqrt(x*x + y*y);
					float f1 = 0;
					for(float r0 = 0; r0 < 1.0f; r0+= 0.1f)
					{
						float dr = 0.1f - fabs(f - r0);
						f1 = E_MAX(f1, dr);
					}

					f1 = f1 * 5;

					if(f > 0.95f)
					{
						f = (1.0f - f) * 20.0f;
						if(f < 0)
						{
							f = 0;
						}
					}

					float r = inner.r + (outer.r - inner.r) * f;
					float g = inner.g + (outer.g - inner.g) * f;
					float b = inner.b + (outer.b - inner.b) * f;
					float a = inner.a + (outer.a - inner.a) * f;

					r = r + (1.0f - r) * f1;
					g = g + (1.0f - g) * f1;
					b = b + (1.0f - b) * f1;
					a = a + (1.0f - a) * f1;

					p[0] = (int)(r * 255);
					p[1] = (int)(g * 255);
					p[2] = (int)(b * 255);
					p[3] = (int)(a * 255);
				}
			}
			return true;
		}
	};

	class ProcImagePlayerAura : public TexLoader
	{
		uint w, h;
		RGBA color;
	public:
		ProcImagePlayerAura(uint _w, uint _h, const RGBA & _color)
		{
			w = _w;
			h = _h;
			color = _color;
		}

		bool Load(Image & _pic) override
		{
			_pic.Alloc(w, h);
			float hw = w * 0.5f;
			float hh = h * 0.5f;

			Vector2 v;
			for(int i=0; i<(int)w; i++)
			{
				for(int j=0; j<(int)h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					v.x = (i - hw) / hw;
					v.y = (j - hh) / hh;
					float r = v.length();
					if(r < 0.75f)
					{
						p[0] = p[1] = p[2] = p[3] = 0;
						continue;
					}
					float a = v.Angle();
					float da = sin(a*36) + 1.0f;

					float dr = r - 0.90f;
					dr = fabs(dr);
					dr = 1.0f - dr * 10.0f;
					if(dr < 0)
					{
						dr = 0;
					}
					else if(dr > 1.0f)
					{
						dr = 1.0f;
					}

					float f = dr * da * 0.75f;

					p[0] = (int)(color.r * 255);
					p[1] = (int)(color.g * 255);
					p[2] = (int)(color.b * 255);
					p[3] = (int)(color.a * f * 255);
				}
			}
			return true;
		}
	};

	class ProcImageStripA : public TexLoader
	{
	public:
		bool Load(Image & _pic) override
		{
			uint _w = 256;
			uint _h = 32;
			_pic.Alloc(_w, _h);
			float hh = _h * 0.5f;
			float ar = 0;
			float ag = PI * 2.0f / 3.0f;
			float ab = PI * 4.0f / 3.0f;
			float ry = hh * 1.0f;
			for(int i=0; i<(int)_w; i++)
			{
				float a = PI * 2.0f * float(i) / float(_w);
				float yr = hh + cos((a + ar) * 5.0f) * ry;
				float yg = hh + cos((a + ag) * 3.0f) * ry;
				float yb = hh + cos((a + ab) * 1.0f) * ry;

				for(int j=0; j<(int)_h; j++)
				{
					uint8 * p = _pic.Get(i, j);
					//float dy = (j - (float)_h) / (float)_h;
					float f = j / (float)_h;
					float fr = 1.0f-0.04f*fabs(j - yr);
					float fg = 1.0f-0.03f*fabs(j - yg);
					float fb = 1.0f-0.02f*fabs(j - yb);
					if(fr > 1) fr = 1;
					if(fr < 0) fr = 0;
					if(fg > 1) fg = 1;
					if(fg < 0) fg = 0;
					if(fb > 1) fb = 1;
					if(fb < 0) fb = 0;
					//f = (fr+fg+fb) > 0 ? 1.0f : 0.0f;
					p[0] = (int)(fr * 255);
					p[1] = (int)(fg * 255);
					p[2] = (int)(fb * 255);
					p[3] = (int)(f * f * 160);
				}
			}
			return true;
		}
	};

	class ProcImageStripLing : public TexLoader
	{
	public:
		bool Load(Image & _pic) override
		{
			uint _w = 256;
			uint _h = 32;
			_pic.Alloc(_w, _h);
			float da = 15.0f * PI / _w;
			float hh = _h * 0.5f;
			for(int i=0; i<(int)_w; i++)
			{
				float a = i * da;
				float ax = sin(a);
				ax = fabs(ax);
				ax*= 0.9f;
//				if(ax >= 0.9f)
//				{
//					ax = 0.9f;
//				}
				for(int j=0; j<(int)_h; j++)
				{
					float ay = float(j - hh) / hh;
					ay = 1.0f - fabs(ay);
					ay*= 1.5f;
					if(ay > 0.9f)
					{
						ay = 0.9f;
					}
					float a = ax * ay;
					uint8 * p = _pic.Get(i, j);
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
					p[3] = (int)(a * 255);
				}
			}
			return true;
		}
	};
}

