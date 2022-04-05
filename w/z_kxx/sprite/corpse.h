
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class Corpse : public Sprite
	{
	public:
		float tx0;
		float ty0;
		float tx1;
		float ty1;
		float rot;
		Vector2 vel;
		Vector2 acc;
		uint32 timer;
		float alpha_delta;
		float scale_delta;
		bool Step() override;
		void Render() override;
	};
}

