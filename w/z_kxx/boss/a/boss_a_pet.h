#pragma once

#include <z_kxx/enemy/enemy.h>

namespace e
{
	class BossAPet : public Enemy
	{
		uint32 shot_timer;
		Vector2 velocity;
		float target_a;
		float target_b;
		Vector2 delay_master_pos;
	public:
		int state;
		BossAPet(const Vector2 & _pos);
		bool Step() override;
	};

}

