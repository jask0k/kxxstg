
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class CometTail : public Sprite
	{
		struct Particle
		{
			int life;
			float x;
			float y;
			float dx;
			float dy;
			float size;
			struct
			{
				float r;
				float g;
				float b;
				float a;
			}clr;
		};
		typedef List<Particle> ParticleList;
		ParticleList particles;
	public:
		void Add(const Vector2 & _pt, float _speed, float _angle);
		bool Step() override;
		void Render() override;
	};

	class CometShot : public EnemyShot
	{
		CometTail * tail;
		uint32 add_timer;
		uint32 durationTimer;
	public:
		float speed;
		float angleDelta;
		CometShot();
		~CometShot();
		bool Step() override;
		void Render() override;
		//bool OnHit(const Vector2 & _pt) override;
		void DropItem() override;

	};
}

