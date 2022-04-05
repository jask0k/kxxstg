#pragma once

#include <z_kxx/boss/boss_script.h>

namespace e
{
	class BossASC1 : public SCScript
	{
		int throw_timer;
		Vector2 star[5];
		int star_index;
		int state;
		uint32 timer;
		Vector2 vel;
		void CalcState0();
		float throw_a;
		float throw_da;
	public:
		BossASC1();
		void Begin() override;
		bool Step() override;
		void End() override;
	};
}

