
// #include "../config.h"
#include <z_kxx/util/ani.h>
#include <z_kxx/sprite/ani_spark.h>

namespace e
{
	AniSpark::AniSpark(const Ani & _ani, float _angle0, float _angle1, float _scale0, float _scale1, float _alpha0, float _alpha1)
	{
		ani = _ani;
		tex = ani.GetTex();
		hsz.x = tex->W() * 0.5f;
		hsz.y = tex->H() * 0.5f;
		timer = duration = _ani.GetTotalSpan();
		E_ASSERT(duration > 0);
		ang  = _angle0;
		angleDelta = (_angle1 - _angle0) / duration;
		scl.x =  _scale0;
		scaleDelta = (_scale1 - _scale0) / duration;
		clr.a =  _alpha0;
		alphaDelta = (_alpha1 - _alpha0) / duration;
	}


	AniSpark::~AniSpark()
	{
	}

	void AniSpark::Render()
	{
		if(!tex)
		{
			return;
		}

		Sprite::Render();

	}

	bool AniSpark::Step()
	{
		//if(_mf)
		//{
		//	return true;
		//}

		if(--timer == 0)
		{
			timer = 1;
			return false;
		}

		pos+= speed;
		float ratio = 1.0f - (float)timer / (float)duration;
		this->ang+= angleDelta;
		this->clr.a+= alphaDelta;
		this->scl.x+= scaleDelta;
		this->scl.y = scl.x;
		ani.Step();
		tex = ani.GetTex();
		return true;
	}

}
