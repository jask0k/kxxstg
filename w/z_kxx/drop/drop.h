
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class Drop : public Sprite
	{
	public:
		int dropType;
		enum
		{
			MANUAL_ABSORB,
			AUTO_ABSORB,
			NO_ABSORB,
		}absorb_mode;
		TexRef tex1; // glow
		Vector2 speed;
		float angle_delta;
		float absorbSpeed;
		bool absorbing; 
		float max_vel;
		void Render() override;
		bool Step() override;
	};

	typedef List<Drop*> DropList;
}
