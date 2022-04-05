#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
//	class BossASC4MasterShot;
	class BossASC4 : public SCScript
	{
		uint32 shot_timer;
		uint32 shot2_timer;
	public:
		BossASC4();
		void Begin() override;
		bool Step() override;
		void End() override;
	};
}

