
// #include "../config.h"
#include <z_kxx/shot/lapillus_shot.h>
#include <z_kxx/sprite/spark1.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	LapillusShot::LapillusShot()
	{
		smokeTex = kxxwin->LoadTex("moon-red");
		tex = kxxwin->shotTex[3][0];
		//tex->AddRef();
		fadeIn.Set(240);
		hsz.x = hsz.y = 32;
		t1 = 1;
		gravity.y = 0.002f;
	}

	LapillusShot::~LapillusShot()
	{
		//if(smokeTex)
		//{
		//	smokeTex->Release();
		//}

		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void LapillusShot::Render()
	{
		this->RotateNorthToVectorDirection(speed);
		clr.a = fadeIn.Ratio();
		Sprite::Render();
	}

	bool LapillusShot::Step() 
	{
		if(fadeIn.Step())
		{
			t1--;
			if(t1 == 0)
			{
				t1 = 30;
				// ��һ����
				float f = fadeIn.Ratio();
				float r = 32 * (1 - f) + 5;
				Spark1 * s = enew Spark1(smokeTex, 0.3f, 0, 0, r, r , f*0.8f , f*0.2f);
				s->pos = this->pos;
				s->clr.r = s->clr.g = s->clr.b = f;
				kxxwin->AddSparkToList(s, RL_GAME_ENEMY_SHOT);
			}
		}
		pos+= speed;
		speed+= gravity;
		return true;
	}
}
