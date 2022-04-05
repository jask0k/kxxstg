
#pragma once
#include <z_kxx/sprite/sprite.h>

namespace e
{
	class AniSpark : public Sprite
	{
		float angleDelta;
		float scaleDelta;
		float alphaDelta;
		Ani ani;
		uint timer;
		uint duration;
	public:
		Vector2 speed;
		AniSpark(const Ani & _ani, float _angle0, float _angle1, float _scale0, float _scale1, float _alpha0 = 1, float _alpha1 = 1);
		~AniSpark();
		void Render() override;
		bool Step() override;
	};
}


