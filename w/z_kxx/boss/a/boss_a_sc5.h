#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
	class BossASC5 : public SCScript
	{
		uint32 shot_timer;
	public:
		BossASC5();
		void Begin() override;
		bool Step() override;
		void End() override;
	};
}

