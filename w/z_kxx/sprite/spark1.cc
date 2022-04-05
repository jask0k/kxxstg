
// #include "../config.h"
#include <z_kxx/sprite/spark1.h>

namespace e
{
	Spark1::Spark1(TexRef _tex, float _second, float _angle0, float _angle1, float _radius0, float _radius1, float _alpha0, float _alpha1)
	{
		tex = _tex;
		//if(tex)
		//{
		//	tex->AddRef();
		//}
		speed.x = 0;
		speed.y = 0;
		float t = _second * K_LOGIC_FPS;
		counter.Set(t);
		angle0  = _angle0;
		angleDelta = (_angle1 - _angle0);
		radius0 =  _radius0;
		radiusDelta = (_radius1 - _radius0);
		alpha0 =  _alpha0;
		alphaDelta = (_alpha1 - _alpha0);
	}


	Spark1::~Spark1()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void Spark1::Render()
	{
		if(!tex)
		{
			return;
		}
		float ratio = counter.Ratio();
		this->ang = angle0 + ratio * angleDelta;
		this->clr.a = alpha0 + ratio * alphaDelta;
		this->hsz.y = this->hsz.x = radius0 + ratio * radiusDelta;

		Sprite::Render();

	}

	bool Spark1::Step()
	{
		pos+= speed;
		return counter.Step();
	}

}
