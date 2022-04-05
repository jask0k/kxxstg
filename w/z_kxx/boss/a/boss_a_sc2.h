#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
	class BossASC2 : public SCScript
	{
		Vector2 start_pos;
		Vector2 vel;
		uint32 shot_timer;
		int state;
		uint32 state2_timer;
	public:
		BossASC2();
		void Begin() override;
		bool Step() override;
		void End() override;
		void CalcState0Vel();
		void CastPetShot();
	};
}

