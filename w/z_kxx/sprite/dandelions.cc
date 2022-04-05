
// #include "../config.h"
#include <z_kxx/sprite/dandelions.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Dandelions::Dandelions()
	{
		particles = enew PARTICLE[particleCount];
		tex = kxxwin->LoadTex("dandelion");
		particleTimer;
		pendingParticle;
		particleTimer = 0;
		pendingParticle = 0;
		for(int i=0; i<particleCount; i++)
		{
			PARTICLE & p = particles[i];
			p.isRunning = false;
		}
		blowAway = false;
		isRunning = true;
	}

	Dandelions::~Dandelions()
	{
		delete[] particles;
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void Dandelions::Render()
	{
		// graphics->BlendOn();
		graphics->SetTexMode(TM_MODULATE);
		//// graphics->SetTexMode(TextureMode::Modulate);
		//graphics->SetBlendMode(BM_ADD)
		graphics->BindTex(tex);
	//	graphics->SetColor(1, 1, 1, 1);

		graphics->BindTex(tex);
		bool renderAny = false;
		for(int i=0; i<particleCount; i++)
		{
			PARTICLE & p = particles[i];
			if(p.isRunning)
			{
				renderAny = true;
				graphics->PushMatrix();
				graphics->TranslateMatrix(p.x, p.y, 0);
				float a = p.dx * 0.3f;
				graphics->RotateMatrix(a, 0, 0, 1);
				graphics->DrawQuad(-p.r, -p.r, p.r, p.r);
				graphics->PopMatrix();
				p.x+= p.dx;
				p.y+= p.dy;
				if(p.y < - 20)
				{
					p.isRunning = false;
				}

				// big one is near, move to list tail
				if(i > 0 && particles[i-1].r > p.r)
				{
					PARTICLE t = p;
					p = particles[i-1];
					particles[i-1] = t;
				}
			}
		}

		if(!renderAny && blowAway)
		{
			isRunning = false;
		}

	//	graphics->SetBlendMode(BM_NORMAL);
	//	// graphics->SetTexMode(TextureMode::replace);
	}
	

	bool Dandelions::Step()
	{
		if(!isRunning)
		{
			return false;
		}

		if(particleTimer)
		{
			particleTimer--;
		}
		else
		{
			particleTimer = 2;
			PARTICLE & p = particles[pendingParticle];
			pendingParticle++;
			if(pendingParticle >= particleCount)
			{
				pendingParticle = 0;
			}
			if(p.isRunning)
			{
				p.dx+= (rand() % 201 - 100) * 0.003f;
			}
			else if(!blowAway)
			{
				float n = float(rand() % 100);
				p.r = (n*n / 100) * 0.30f + 7.0f;
				p.dy = -((rand() % 100) * 0.005f + p.r * 0.025f);
				p.dx = (rand() % 100 ) * 0.005f;
				//p.a = 0;
				//p.da = 0;// (rand() % 201 - 100) * 0.0005f + 0.01f;
				p.isRunning = true;
				p.y = K_VIEW_H + 20;
				p.x = float(rand() % 1200) - 400;
				//p.left = true;
			}
		}


		int n = rand() % 200;
		if(n == 0 || n == 1)
		{
			float ddx = 0.1f * (rand() % 10);
			for(int i = 0; i < particleCount; i++)
			{
				PARTICLE & p = particles[i];
				if(p.isRunning && ((n==0) == (p.r < 14.0f)))
				{
					p.dx+= ddx * p.r * 0.01f;
				}
			}
		}	

		return true;
	}

	void Dandelions::BlowAway()
	{
		blowAway = true;

		for(int i = 0; i < particleCount; i++)
		{
			PARTICLE & p = particles[i];
			if(p.isRunning)
			{
				p.dx+= -18.0f;
				p.dy+= -12.0f;
			}
		}
	}

}
