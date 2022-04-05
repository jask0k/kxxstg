
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class AccelerateShot : public EnemyShot
	{
		uint32 timer1;
		uint32 timer2;
		float r;
		float dr;
		TexRef tex1;
		TexRef tex2;
	public:
		Vector2 speed;
		Vector2 accel;
		AccelerateShot(TexRef _tex, const Vector2 & _speed0, const Vector2 & _speed1, float _second);
		AccelerateShot(TexRef _tex, const Vector2 & _speed0, const Vector2 & _speed1, float _second
			, TexRef _smokeTex, float _r0, float _r1, float _second1);
		~AccelerateShot();
		bool Step() override;
		void Render() override;
		bool Collide(const Vector2 & _v, float _r) override;
	};
}


