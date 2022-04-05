
// #include "../config.h"
#include <z_kxx/shot/laser_fan.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
#define MIN_DISTANCE 30
#define MAX_DISTANCE 500
#define THRESHOLD 0.6f
	LaserFan::LaserFan(const Vector2 & _pt, int _split)
		: SPLIT(_split)
	{
		fragile = false;
		laser[0].resize(_split);
		laser[1].resize(_split);
		pos = _pt;
		glowTex = kxxwin->LoadTex("laser-glow");
		tex0 = kxxwin->LoadTex("laser-blue");
		tex1 = kxxwin->LoadTex("laser-red");
		timerMax = 240;
		timer = -30;
		float da = 2.0f * PI / SPLIT;
		for(int i = 0; i < SPLIT; i++)
		{
			for(int j=0; j<2; j++)
			{
				Laser & l = laser[j][i];
				l.size = 0;
				l.ang =  da * i - PI *0.5f + j*da*0.5f;
				float sin_a = sin(l.ang);
				float cos_a = cos(l.ang);
				l.p0.x = pos.x + MIN_DISTANCE * cos_a;
				l.p0.y = pos.y + MIN_DISTANCE * sin_a;
				l.p1.x = pos.x + MAX_DISTANCE * cos_a;
				l.p1.y = pos.y + MAX_DISTANCE * sin_a;
			}
		}
	}


	LaserFan::~LaserFan()
	{
		//if(tex1)
		//{
		//	tex1->Release();
		//}
		//if(tex0)
		//{
		//	tex0->Release();
		//}
		//if(glowTex)
		//{
		//	glowTex->Release();
		//}
	}

	void LaserFan::Render()
	{
		graphics->SetTexMode(TM_MODULATE);
		// graphics->BlendOn();
		// graphics->SetTexMode(TextureMode::Modulate);
		float da = PI / SPLIT;
		float xc = pos.x;
		float yc = pos.y;
		graphics->PushMatrix();
		graphics->TranslateMatrix(xc, yc, 0);
		graphics->RotateMatrix(PI *0.5f, 0, 0, -1);
		for(int i = 0; i < SPLIT; i++)
		{
			if(laser[0][i].size > 0)
			{
				graphics->BindTex(tex1);
				graphics->SetColor(1, 1, 1, laser[0][i].size);
				graphics->DrawQuad(MIN_DISTANCE, -laser[0][i].size*4, MAX_DISTANCE, laser[0][i].size*4);
				graphics->BindTex(glowTex);
				graphics->DrawQuad(MIN_DISTANCE*0.5f, -laser[0][i].size*16, MIN_DISTANCE * laser[0][i].size*8, laser[0][i].size*16);
			}
			graphics->RotateMatrix(-da, 0, 0, -1);
			if(laser[1][i].size > 0)
			{
				graphics->BindTex(tex0);
				graphics->SetColor(1, 1, 1, laser[1][i].size);
				graphics->DrawQuad(MIN_DISTANCE, -laser[1][i].size*4, MAX_DISTANCE, laser[1][i].size*4);
				graphics->BindTex(glowTex);
				graphics->DrawQuad(MIN_DISTANCE*0.5f, -laser[1][i].size*16, MIN_DISTANCE * laser[1][i].size*8, laser[1][i].size*16);
			}
			graphics->RotateMatrix(-da, 0, 0, -1);
		}
		graphics->PopMatrix();
		// graphics->SetTexMode(TextureMode::replace);
	}

	bool LaserFan::Step()
	{
		//if(_mf < 2)
		//{
		//	return true;
		//}
		timer++;
		float a0 = 2.0f * float(timer) * PI / timerMax;
		for(int i = 0; i < SPLIT; i++)
		{
			float da = a0 + laser[0][i].ang - PI *0.5f;
			if(da >= 2.5f*PI && da < 8.5f*PI)
			{
				float f = sin(da-PI);
				laser[0][i].size = (f + 1.0f) * 0.5f;
			}
			else if(da >= 10.5f*PI)
			{
				laser[0][i].size = 0;
				return false;
			}
			else
			{
				laser[0][i].size = 0;
			}
			da = a0 + laser[1][i].ang - PI *0.5f;
			if(da >= 1.5f*PI && da < 7.5f*PI)
			{
				float f = sin(da);
				laser[1][i].size = (f + 1.0f) * 0.5f;
			}
			else
			{
				laser[1][i].size = 0;
			}
		}

		return true;
	}

	bool LaserFan::Collide(const Vector2 & _v, float _r)
	{
		float tmp;
		for(int i = 0; i < SPLIT; i++)
		{
			for(int j=0; j<2; j++)
			{
				Laser & l = laser[j][i];
				if(l.size >= THRESHOLD)
				{
					float dis = PointLineSegDistanceSquared2D(_v, l.p0, l.p1, tmp);
					//if(i == SPLIT/2)
					//{
					//	E_TRACE_LINE("dis = " + string(dis));
					//}
					if(dis <= _r * _r)
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	void LaserFan::DropItem()
	{
		for(int i = 0; i < SPLIT; i++)
		{
			for(int j=0; j<2; j++)
			{
				Laser & l = laser[j][i];
				if(l.size >= THRESHOLD)
				{
					float dv_x = (l.p1.x - l.p0.x)*0.1f;
					float dv_y = (l.p1.y - l.p0.y)*0.1f;
					for(int k=0; k<=10; k++)
					{
						Vector2 v = {l.p0.x + dv_x * k, l.p0.y + dv_y * k};
						//kxxwin->AddSmallSpark(this, 1);
						kxxwin->AddDrop(DROP_TINY_POINT, v.x, v.y, false, -(PI * 0.5f));
					}
				}
			}
		}
	}



}
