
// #include "../config.h"
#include <z_kxx/sprite/galaxy.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Galaxy::Galaxy()
	{
		tex = kxxwin->LoadTex("gather-star");
		num = 0;
	}

	Galaxy::~Galaxy()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void Galaxy::Render()
	{
		// graphics->BlendOn();
		graphics->SetTexMode(TM_MODULATE);
		// graphics->SetTexMode(TextureMode::Modulate);
		graphics->SetBlendMode(BM_ADD);
		graphics->BindTex(tex);

		ParticleList::iterator it = particles.begin();
		ParticleList::iterator end = particles.end();
		for(;it != end; ++it)
		{
			Particle & p = *it;
			graphics->SetColor(p.clr.r, p.clr.g, p.clr.b, p.clr.a);
			float x = pos.x + cos(p.ang) * p.radius;
			float y = pos.y + sin(p.ang) * p.radius;
			graphics->DrawQuad(x - p.size, y - p.size, x + p.size, y + p.size);
		}
		graphics->SetBlendMode(BM_NORMAL);
		// graphics->SetTexMode(TextureMode::replace);
	}
	
	void Galaxy::Add(uint _n)
	{
		num+= _n;
	}

	bool Galaxy::Step()
	{
		ParticleList::iterator it = particles.begin();
		ParticleList::iterator end = particles.end();
		while(it != end)
		{
			Particle & p = *it;
			if(--p.life)
			{
				if(p.radius > 30)
				{
					p.radius-=1.2f;
					p.clr.a= 1.0f - p.radius / 400.0f;
				}
				p.ang+= 0.02f;
				++it;
			}
			else
			{
				it = particles.erase(it);
			}
		}
		//if(_mf & 0x01)
		//{
		//	return true;
		//}
		uint add = 8;
		float f;
		while(num && add--)
		{
			Particle p;
			f = frand();
			p.ang  = frand() + add * PI * 0.5f;
			p.radius = f * 30 + 300;
			p.clr.r = frand() * 0.5f + 0.5f;
			p.clr.g = frand() * 0.5f + 0.5f;
			p.clr.b = frand() * 0.5f + 0.5f;
			p.clr.a = 0;
			p.life = 250;
			p.size = f * f * 8 + 2;
			particles.push_back(p);
			num--;
		}
		//if(num == 0 && particles.empty())
		//{
		//	isRunning = false;
		//}
		return true;
	}
}
