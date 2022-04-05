
#pragma once

#include <z_kxx/sprite/sprite.h>
#include <nbug/tl/list.h>

namespace e
{
	class Galaxy : public Sprite
	{
		struct Particle
		{
			float ang;
			float radius;
			float size;
			struct
			{
				float r;
				float g;
				float b;
				float a;
			}clr;
			int life;
		};
		typedef List<Particle> ParticleList;
		ParticleList particles;
		uint num;
	public:
		Galaxy();
		~Galaxy();
		void Add(uint _n);
		void Render() override;
		bool Step() override;
	};
}

