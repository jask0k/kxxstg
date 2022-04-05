
// #include "../config.h"
#include <z_kxx/shot/comet_shot.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{

	bool CometTail::Step()
	{
		ParticleList::iterator it = particles.begin();
		ParticleList::iterator end = particles.end();
		while(it != end)
		{
			Particle & p = *it;
			if(--p.life)
			{
				p.clr.a-= 0.004f;
				p.x+= p.dx;
				p.y+= p.dy;
				++it;
			}
			else
			{
				it = particles.erase(it);
			}
		}

		return !particles.empty();
	}

	void CometTail::Render()
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
			float x = p.x;
			float y = p.y;
			graphics->DrawQuad(x - p.size, y - p.size, x + p.size, y + p.size);
		}
		graphics->SetBlendMode(BM_NORMAL);
		// graphics->SetTexMode(TextureMode::replace);
	}
	
	void CometTail::Add(const Vector2 & _pt, float _speed, float _angle)
	{
		Particle p;
		float f = frand();
		_angle =  _angle + PI + (frand() - 0.5f) * 0.8f;
		_speed = _speed * 4 *0.5f;
		p.dx = cos(_angle) * _speed;
		p.dy = sin(_angle) * _speed;
		p.x = _pt.x + rand() % 32 - 16;
		p.y = _pt.y + rand() % 32 - 16;
		p.clr.r = frand() * 0.5f + 0.5f;
		p.clr.g = frand() * 0.5f + 0.5f;
		p.clr.b = frand() * 0.5f + 0.5f;
		p.clr.a = 1.0f;
		p.life = 59 * K_LOGIC_FPS_MUL;
		p.size = f * f * 16 + 4;
		particles.push_back(p);
	}


	CometShot::CometShot()
	{
		int h[5] = {3, 4, 5, 9, 10};
		tex = kxxwin->shotTex[3][h[rand()%5]];
		speed = 0;
		angleDelta = 0;
		durationTimer = S2F(3);
		tail = enew CometTail();
		tail->tex = kxxwin->flashTex;
		add_timer = K_LOGIC_FPS_MUL;
	}


	CometShot::~CometShot()
	{
		// tail still live 
		kxxwin->AddSparkToList(tail, RL_GAME_ENEMY_SHOT);
	}


	void CometShot::Render()
	{
		EnemyShot::Render();
		tail->Render();
	}

	bool CometShot::Step()
	{
		if(--add_timer == 0)
		{
			tail->Add(pos, speed, ang);
			add_timer = K_LOGIC_FPS_MUL;
		}
		tail->Step();

		pos.x+= speed * cos(ang);
		pos.y+= speed * sin(ang);

		// angle to player
		const Vector2 & c = pos;
		float a = (kxxwin->player->pos-c).Angle();

		// angle delta
		float da;
		if(durationTimer)
		{
			durationTimer--;
			da = angle_normalize(a - ang);
			da = angle_limit(da, 0.001f * PI);
		}
		else
		{
			da = 0;
		}

		// enew angle
		float a1 = ang + da;
		float a2 = angle_normalize((ang + a1) * 0.5f);

		ang = a1;

		return !IsOutOfGameArea(pos, 40);
	}

	//bool CometShot::OnHit(const Vector2 & _pt)
	//{
	//	kxxwin->AddExplosion(_pt, 40, hue);
	//	return true;
	//}

	void CometShot::DropItem()
	{
		for(float a = 0; a < 2*PI; a+= 1.257f)
		{
			kxxwin->AddDrop(DROP_TINY_POINT, pos.x + cos(a) * this->hsz.x, pos.y + sin(a) * this->hsz.y, false, -(PI * 0.5f));
		}
	}

}
