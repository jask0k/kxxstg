
#pragma once

#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani5.h>

namespace e
{
	class WildFlower : public Enemy
	{
		int state;
		uint timer;
		Vector2 vel;
		Vector2 acc;
		int shot_skip;
	public:
		WildFlower(int _pos, int _drop);
		~WildFlower();
		bool Step() override;
	};

}

