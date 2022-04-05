#pragma once
#include <z_kxx/boss/boss_script.h>
#include <z_kxx/boss/b/boss_b.h>

namespace e
{
	class BossBNA1 : public NAScript
	{
	public:
		Ani ani;
		uint timer;
		int state;
		BossBNA1();
		void Begin() override;
		bool Step() override;
		BossB * pb()
		{ return dynamic_cast<BossB*>(boss); }
		void InitState0();
		void InitState1();
	};
}

