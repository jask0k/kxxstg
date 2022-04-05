
#pragma once

#include <z_kxx/sprite/sprite.h>
#include <nbug/tl/list.h>

namespace e
{
	class Dandelions : public Sprite
	{
		struct PARTICLE
		{
			bool isRunning;
			float x;
			float y;
			float dx;
			float dy;
			float r;
		};
		uint32 particleTimer;
		int pendingParticle;
		static const int particleCount = 160;
		PARTICLE * particles;
		bool blowAway;
		bool isRunning;
	public:
		Dandelions();
		~Dandelions();
		void Render() override;
		bool Step() override;
		void BlowAway();
	};
}

