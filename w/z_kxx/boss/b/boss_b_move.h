#pragma once

#include <z_kxx/boss/boss_script.h>
#include <z_kxx/boss/b/boss_b.h>

namespace e
{
	class BossBMove : public BossScript
	{
		Vector2 to;
		float vel_f;
		Vector2 vel;
		uint32 timer;
		Ani ani;
		bool stand;
	public:
		BossBMove(uint32 _time, float _to_x, float _to_y);
		void Begin() override;
		bool Step() override;
		//BossB * pb()
		//{ return dynamic_cast<BossB*>(boss); }
	};
}

