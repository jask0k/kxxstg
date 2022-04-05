
// #include "../config.h"
#include <nbug/gl/graphics.h>
#include <z_kxx/effect/lens_effect.h>

namespace e
{
	static float dx = 0;
	static float dy = 0;

	LensEffect::LensEffect()
	{
		g = 0;
		fbo = 0;
		timer = 0;
	}

	inline static int MKIDX(int i, int j)
	{
		return i * GRIDH + j;
	}

	void LensEffect::Init(Graphics * _g, float _x, float _y, float _w, float _h)
	{
		E_ASSERT(g == 0);
		g = _g;
		pos.x = _x;
		pos.y = _y;
		w = _w;
		h = _h;

		E_ASSERT(g->GetCapabilities().frameBufferObject);
		fbo = g->CreateFbo((int)w, (int)h);

		dx = float(w) / (GRIDW-1);
		dy = float(h) / (GRIDH-1);
		float tx = 1.0f / (GRIDW-1);
		float ty = 1.0f / (GRIDH-1);
		for(int i=0; i<GRIDW; i++)
		{
			for(int j=0; j<GRIDH; j++)
			{
				MASS & m = grid2[i][j];
				m.pos.x = i * dx;
				m.pos.y = j * dy;
				m.pos0 = m.pos;
				m.tc.x = i * tx;
				m.tc.y = j * ty;
				m.fixed = i==0 || j==0 ||i == GRIDW-1 || j == GRIDH-1;
				m.v.x = 0;
				m.v.y = 0;
				m.f.x = 0;
				m.f.y = 0;
			}
		}

		float k1 = 0.1f;
		float k2 = 0.1f;

		//BUG: missing the point at bottom right corner
		for(int i=0; i<GRIDW-1; i++)
		{
			for(int j=0; j<GRIDH-1; j++)
			{
				int i1 = i+1;
				int j1 = j+1;
				// horiz
				Spring b;
				b.a = MKIDX(i, j);
				b.b = MKIDX(i1, j);
				b.k = k1;
				b.l = dx;
				spring1.push_back(b);

				// vert
				b.a = MKIDX(i, j);
				b.b = MKIDX(i, j1);
				b.k = k1;
				b.l = dy;
				spring1.push_back(b);
			}
		}

		//
		float len = sqrt(dx*dx+dy*dy);
		for(int i=0; i<GRIDW-1; i++)
		{
			for(int j=0; j<GRIDH-1; j++)
			{
				int i1 = i+1;
				int j1 = j+1;
				// horiz
				Spring b;
				b.a = MKIDX(i, j);
				b.b = MKIDX(i1, j1);
				b.k = k2;
				b.l = len;
				spring1.push_back(b);

				// vert
				b.a = MKIDX(i, j1);
				b.b = MKIDX(i1, j);
				b.k = k2;
				b.l = len;
				spring1.push_back(b);
			}
		}
	}


	LensEffect::~LensEffect()
	{
		//if(fbo)
		//{
		//	fbo->Release();
		//}
	}

#define RRR 150.0f
	void LensEffect::Step(float _x, float _y)
	{
		timer++;
		Vector2 dv = {_x, _y};
		dv = dv - pos;
		pos.x = _x;
		pos.y = _y;
		const Vector2 & v0 = pos;

		Vector2 v2;

		for(int i=0; i<GRIDSIZE; i++)
		{
			MASS & m = grid1[i];

			// let m = 1, so a = f
			m.v.x = m.v.x + m.f.x;
			m.v.y = m.v.y + m.f.y;

			// calc enew pos
			if(!m.fixed)
			{
				m.pos.x = m.pos.x + m.v.x;  
				m.pos.y = m.pos.y + m.v.y; 
			}			

			//
			v2 = v0 - m.pos0;
			float r = v2.length();
			if(r < RRR) // control range
			{
				
				float f = (RRR - r) / RRR;

				float a = v2.Angle();
				r = (RRR - r) / RRR;
			//	float b = sin(timer * 0.01f);
				//r =  r + r * b * 0.4f;
				r = sqrt(r);
				if(r > 0.99f)
				{
					r = 0.99f;
				}

				m.f.x = - r * (cos(a) * 0.12f + cos(timer * 0.04f) * 0.015f) + dv.x *0.005f*r;
				m.f.y = - r * (sin(a) * 0.12f + sin(timer * 0.06f) * 0.015f) + dv.y *0.005f*r;
			}
			else 
			{
				r = (r - RRR) / 50.0f;
				if(r > 1.0f)
				{
					r = 1.0f;
				}
				m.f.x = 0;
				m.f.y = 0;
				m.pos.x += (m.pos0.x - m.pos.x) * 0.4f * r;
				m.pos.y += (m.pos0.y - m.pos.y) * 0.4f * r;
			}
		}

		// damping
		//for(auto it = spring1.begin(); it != spring1.end(); ++it)
		//{
		//	Spring & s = *it;
		//	MASS & ma = grid1[s.a];
		//	MASS & mb = grid1[s.b];
		//	Vector2 d;
		//	d.x = (mb.pos.x - ma.pos.x);
		//	if(d.x < 0)
		//	{
		//		continue;
		//	}
		//	d.y = (mb.pos.y - ma.pos.y);
		//	if(d.y < 0)
		//	{
		//		continue;
		//	}
		//	float l = d.length();
		//	float lmax = s.l * 1.2f;
		//	if(l > lmax)
		//	{
		//		l = l - lmax;
		//		float a = d.Angle();
		//		const float frac = 0.5f;
		//		if(ma.fixed)
		//		{
		//			mb.pos.x -= l * cos(a)*frac;
		//			mb.pos.y -= l * sin(a)*frac;
		//		}
		//		else if(mb.fixed)
		//		{
		//			ma.pos.x += l * cos(a)*frac;
		//			ma.pos.y += l * sin(a)*frac;
		//		}
		//		else
		//		{
		//			mb.pos.x -= l * cos(a) * 0.5f*frac;
		//			mb.pos.y -= l * sin(a) * 0.5f*frac;
		//			ma.pos.x += l * cos(a) * 0.5f*frac;
		//			ma.pos.y += l * sin(a) * 0.5f*frac;
		//		}
		//	}
		//}

		//
		for(List<Spring>::iterator it = spring1.begin(); it != spring1.end(); ++it)
		{
			Spring & s = *it;
			MASS & ma = grid1[s.a];
			MASS & mb = grid1[s.b];
			Vector2 d;
			d.x = (mb.pos.x - ma.pos.x);
			d.y = (mb.pos.y - ma.pos.y);
			float l = d.length();
			l = l - s.l;
			float a = d.Angle();

			Vector2 f;
			f.x = l * cos(a) * s.k;
			f.y = l * sin(a) * s.k;

			ma.f.x+= f.x;
			ma.f.y+= f.y;
			mb.f.x-= f.x;
			mb.f.y-= f.y;
		}

		//float dx = float(w) / (GRIDW-1);
		//float dy = float(h) / (GRIDH-1);
		//float tx = 1.0f / (GRIDW-1);
		//float ty = 1.0f / (GRIDH-1);
		//Vector2 v1;
		//Vector2 v2;
		//for(int i=0; i<GRIDW; i++)
		//{
		//	for(int j=0; j<GRIDH; j++)
		//	{
		//		//grid[i][j].pos.x = i * dx;
		//		//grid[i][j].pos.y = j * dy;
		//		//grid[i][j].tc.x = i * tx;
		//		//grid[i][j].tc.y = j * ty;
		//		//float x = i * dx;
		//		//fl0at y = j * dy;
		//		Vector2 & v1 = grid2[i][j].pos;
		//		v1.x = i * dx;
		//		v1.y = j * dy;

		//		v2 = Vector2(v1) - v0;
		//		float a = v2.Angle();
		//		float r = v2.length()*0.1f;
		//		float dr = sin(r);
		//		//r+= dr;
		//		v1.x+= dr * cos(a) * 5.0f;
		//		v1.y+= dr * sin(a) * 5.0f;
		//	}
		//}
	}
	void LensEffect::Render()
	{
		if(!fbo)
		{
			return;
		}

		// g->BlendOff();
		g->SetBlendMode(BM_DISABLE);
		g->SetTexMode(TM_REPLACE);
		// g->SetTexMode(TM_MODULATE);
		g->BindTex(fbo->GetTex());
		// g->SetTexMode(TextureMode::replace);
		//g->DrawQuad(0, 0, (float)w, (float)h);

		for(int i=0; i<GRIDW-1; i++)
		{
			for(int j=0; j<GRIDH-1; j++)
			{
				//float x0 = grid2[i][j].pos0.x;
				//float x1 = grid2[i][j+1].pos0.x;
				//float x2 = grid2[i+1][j+1].pos0.x;
				//float x3 = grid2[i+1][j].pos0.x;
				//float y0 = grid2[i][j].pos0.y;
				//float y1 = grid2[i][j+1].pos0.y;
				//float y2 = grid2[i+1][j+1].pos0.y;
				//float y3 = grid2[i+1][j].pos0.y;

				//---------------------------
				float x0 = grid2[i][j].pos.x;
				float x1 = grid2[i][j+1].pos.x;
				float x2 = grid2[i+1][j+1].pos.x;
				float x3 = grid2[i+1][j].pos.x;
				float y0 = grid2[i][j].pos.y;
				float y1 = grid2[i][j+1].pos.y;
				float y2 = grid2[i+1][j+1].pos.y;
				float y3 = grid2[i+1][j].pos.y;
				//-----------------------------

				float tx0 = grid2[i][j].tc.x;
				float tx1 = grid2[i][j+1].tc.x;
				float tx2 = grid2[i+1][j+1].tc.x;
				float tx3 = grid2[i+1][j].tc.x;
				float ty0 = grid2[i][j].tc.y;
				float ty1 = grid2[i][j+1].tc.y;
				float ty2 = grid2[i+1][j+1].tc.y;
				float ty3 = grid2[i+1][j].tc.y;
				g->DrawQuad(x0, y0, 0, tx0, ty0, x1, y1, 0, tx1, ty1, x2, y2, 0, tx2, ty2, x3, y3, 0, tx3, ty3);
					
			}
		}

		g->BindTex(0);
		// g->BlendOn();
		g->SetBlendMode(BM_NORMAL);
		g->SetTexMode(TM_MODULATE);
		// g->SetTexMode(TM_MODULATE);
	}
}
