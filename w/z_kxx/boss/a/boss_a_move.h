#pragma once

#include <z_kxx/boss/boss_script.h>
#include <z_kxx/boss/a/boss_a.h>

namespace e
{
	class BossAMove : public BossScript
	{
		Vector2 to;
		float vel_f;
		Vector2 vel;
		uint32 timer;
		Ani ani;
		bool stand;
	public:
		BossAMove(uint32 _time, float _to_x, float _to_y);
		void Begin() override;
		bool Step() override;
		//BossA * pb()
		//{ return dynamic_cast<BossA*>(boss); }
	};
}

