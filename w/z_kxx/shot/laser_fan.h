
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class LaserFan : public EnemyShot
	{
	//	float da;
		struct Laser
		{
			Vector2 p0;
			Vector2 p1;
			float ang;
			float size;
		};
		TexRef glowTex;
		TexRef tex0;
		TexRef tex1;
		const int SPLIT;
		Array<Laser> laser[2];
		uint32 timerMax;
		int timer;
	public:
		LaserFan(const Vector2 & _pt, int _split);
		~LaserFan();
		void Render() override;
		bool Step() override;
		bool Collide(const Vector2 & _v, float _r) override;
		void DropItem() override;
	};
}


