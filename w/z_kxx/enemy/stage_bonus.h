
#pragma once

#include <z_kxx/enemy/enemy.h>

namespace e
{
	class StageBonus : public Enemy
	{
		uint32 timer;
		int state;
		uint32 delay;
		uint32 keep_time;
	public:
		StageBonus(TexRef _tex, uint32 _delay, uint32 _total_dealy);
		~StageBonus();
		bool Step() override;
	};
}

