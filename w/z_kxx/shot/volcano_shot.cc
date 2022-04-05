
// #include "../config.h"
#include <z_kxx/shot/volcano_shot.h>
#include <z_kxx/shot/lapillus_shot.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	VolcanoShot::VolcanoShot(int _span)
		: span(_span)
	{
		//tex = kxxwin->LoadTex("flower");
		tex = kxxwin->LoadTex("moon-red");
		t1 = 1;
		t0 = 0;
	}


	VolcanoShot::~VolcanoShot()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void VolcanoShot::Render() 
	{
		float r = t0 > 80 ? float(t0 - 80) * 0.0005f : float(80 - t0) * 0.0083f;
		r = 1 - r;
		hsz.x = 20 + r * 120;
		if(hsz.x < 16)
		{
			hsz.x = 16;
		}
		hsz.y = hsz.x;
		ang = t0 * 0.01f;
		Sprite::Render();
	}

	bool VolcanoShot::Step()
	{
		t0++;
		t1--;
		if(t1 == 0)
		{
			t1 = span;
			float a0 = speed.Angle() - PI * (0.5f + logic_random_float());
			LapillusShot * b = enew LapillusShot();
			b->SetSize(20);
			//b->allowOutOfScreen = false;
			b->isEllipseCollision = true;
			b->collisionFrac.x = 0.35f;
			b->collisionFrac.y = 0.35f;
			b->pos = this->pos;
			b->speed.SetPolar(PS2PF(120), a0);
			b->gravity.x = cos(a0 + PI) * 0.0008f;
			b->gravity.y = sin(a0 + PI) * 0.0008f;
			kxxwin->AddEnemyShotToList(b, false);
		}
		pos+=speed;
		return true;
	}

}

