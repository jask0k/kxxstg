#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
	class BossANA1 : public NAScript
	{
	public:
		uint shot_timer;
		uint shot_counter;
		int state;
		BossANA1();
		void Begin() override;
		bool Step() override;
		void End() override;
	};
}

