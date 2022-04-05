#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
	class BossASC3MasterShot;
	class BossASC3 : public SCScript
	{
		uint32 shot_timer;
		BossASC3MasterShot * ms1;
		BossASC3MasterShot * ms2;
		BossASC3MasterShot * ms3;
		BossASC3MasterShot * ms4;
	public:
		BossASC3();
		void Begin() override;
		bool Step() override;
		void End() override;
	};
}

