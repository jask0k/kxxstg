
#pragma once

#include <z_kxx/shot/accelerate_shot.h>

namespace e
{
	class PetalShot : public EnemyShot
	{
		uint32 timer1;
		uint32 timer2;
		float r;
		float dr;
		TexRef tex1;
		TexRef tex2;
		Vector2 speed;
		Vector2 accel;
	public:
		PetalShot(float _x, float _y, float _a);
		~PetalShot();
		bool Step() override;
		void Render() override;
		bool Collide(const Vector2 & _v, float _r) override;
		void RenderDebug() override;
	};
}


