
// #include "../config.h"
#include <string.h>
#include <z_kxx/shot/worm_shot.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Worm::Worm()
	{
		sectionCount = 0;
		usedSectionCount = 0;
		//spine = 0;
		//vertices = 0;
	}

	void Worm::Init(size_t _sectionCount, float _w, const Vector2 & _pos, float _angle)
	{
		E_ASSERT(_sectionCount > 1);
		sectionCount = _sectionCount;
		usedSectionCount = 1;
		spine.resize(_sectionCount);
		spine[_sectionCount - 1] = _pos;
		vertices.resize(_sectionCount * 2);
		halfWidth = _w * 0.5f;
		angle = _angle;
	}

	Worm::~Worm()
	{
		//delete[] vertices;
		//delete[] spine;
	}

	void Worm::Step(float _dx, float _dy, float _da)
	{
		_da = angle_normalize(_da);

		if(usedSectionCount < sectionCount)
		{
			usedSectionCount++;
		}

		float a1 = angle + _da;
		float a_bisector = (angle + a1) * 0.5f;

		a1 = angle_normalize(a1);

		{
			Vector2 dv;
			dv.SetPolar(halfWidth, a_bisector);
			dv.Prep();
			size_t sz2 = sectionCount * 2;
			const Vector2 & c1 = spine[sectionCount-1];
			vertices[sz2-2] = c1 - dv;
			vertices[sz2-1] = c1 + dv;
		}

		const Vector2 & c = spine[sectionCount-1];
		{
			Vector2 c1 = {c.x + _dx, c.y + _dy};
			Vector2 dv;
			dv.SetPolar(halfWidth, a1);
			dv.Prep();
			_Shift(c1, c1 - dv, c1 + dv);
			angle = a1;
		}
	}

	void Worm::Render(Graphics * _g) 
	{
		if(usedSectionCount > 1)
		{
			Vector2 * p = &vertices[sectionCount * 2 - 2 * usedSectionCount];
			_g->DrawQuadStripY(p, usedSectionCount*2);
		}
	}

	void Worm::RenderDebug(Graphics * _g)
	{
#ifdef NB_DEBUG
		size_t sz = sectionCount;
		size_t n0 = sz - usedSectionCount;
		float r0 = halfWidth;
		RGBA color = kxxwin->debugFontColor;

		for(size_t i = n0; i<sz; i++)
		{
			graphics->DrawEllipse(spine[i].x, spine[i].y, 0, r0, r0, color, false);
		}
#endif
	}

	bool Worm::Collide(const Vector2 & _v, float _r)
	{
		size_t sz = sectionCount;
		size_t n0 = sz - usedSectionCount;
		float r0 = halfWidth + _r;
		r0 = r0 * r0;
		for(size_t i = n0; i<sz; i++)
		{
			float r = (_v - spine[i]).LengthSquared();
			if(r < r0)
			{
				return true;
			}
		}
		return false;
	}

	void Worm::_Shift(const Vector2 & _c, const Vector2 & _l, const Vector2 & _r)
	{
		size_t sz = sectionCount;
		memmove(&spine[0], &spine[1], sizeof(Vector2) * (sz - 1));
		spine[sz-1] = _c;
		sz = sectionCount * 2;
		memmove(&vertices[0], &vertices[2], sizeof(Vector2) * (sz - 2));
		vertices[sz-2] = _l;
		vertices[sz-1] = _r;
	}


	void Worm::DropItem(int _skip, int _hue)
	{
		size_t sz = sectionCount;
		size_t n0 = sz - usedSectionCount;
		for(size_t i = n0; i<sz; i+= _skip)
		{
			kxxwin->AddDrop(DROP_TINY_POINT, spine[i].x, spine[i].y, false, -(PI * 0.5f));
			kxxwin->AddSmallSpark(spine[i], 3, _hue);
		}
	}


	GuideWormShot::GuideWormShot(TexRef _tex, float _w, float _len, const Vector2 & _pt, float _speed, float _angle, float _initialSecond, float _durationSecond)
	{
		tex = _tex;
		if(tex)
		{
			hsz.x = hsz.y = tex->H() * 0.5f;
		}
		wormSectionLen = _speed * 1 * wormStepFrameCount;
		pos = _pt;
		initialTimerMax = initialTimer = S2F(_initialSecond);
		durationTimer = S2F(_durationSecond);

		int sz = int(_len / wormSectionLen + 0.5f) + 1;
		wormStepTimer = wormStepFrameCount;
		worm.Init(sz, _w, _pt, _angle);
	}


	bool GuideWormShot::Step()
	{
		if(initialTimer)
		{
			initialTimer--;
		}
		if(durationTimer)
		{
			durationTimer--;
		}
		wormStepTimer--;
		if(wormStepTimer == 0)
		{
			wormStepTimer = wormStepFrameCount;
			const Vector2 & c = worm.LastPos();
			float a = (kxxwin->player->pos-c).Angle();
			float da;
			if(durationTimer)
			{
				da = angle_normalize(a - worm.angle);
				da = angle_limit(da, 0.03f * PI);
				if(initialTimer)
				{
					da = (initialTimerMax-initialTimer) * da / initialTimerMax;
				}
			}
			else
			{
				da = 0;
			}

			float a1 = worm.angle + da;
			float dx = wormSectionLen * cos(a1);
			float dy = wormSectionLen * sin(a1);

			worm.Step(dx, dy, da);
			pos.x+= dx;
			pos.y+= dy;
		}
		return durationTimer || !worm.Disappear();
	}

	void GuideWormShot::Render()
	{
		graphics->SetColor(clr);
		graphics->BindTex(tex);
		worm.Render(graphics);
	}


	void GuideWormShot::DropItem()
	{
		worm.DropItem(6, hue);
	}

	void GuideWormShot::OnCrash(const Vector2 & _pt)
	{
	}

	bool Worm::Disappear()
	{
		size_t sz = sectionCount;
		size_t n0 = sz - usedSectionCount;
		size_t nn = sz - 1;
		Vector2 & v0 = spine[n0];
		Vector2 & vn = spine[nn];

		float xext = halfWidth * 1.2f;
		float yext = halfWidth * 1.2f;
		return (v0.x < -xext || v0.x > K_GAME_W + xext || v0.y < -yext || v0.y > K_GAME_H + yext) &&
			(vn.x < -xext || vn.x > K_GAME_W + xext || vn.y < -yext || vn.y > K_GAME_H + yext);
	}

	bool GuideWormShot::Collide(const Vector2 & _v, float _r)
	{
		return worm.Collide(_v, _r * collisionFrac.x);
	}

	void GuideWormShot::RenderDebug()
	{
		worm.RenderDebug(graphics);
	}
}
