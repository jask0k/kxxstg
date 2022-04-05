
#pragma once

#include <z_kxx/boss/boss.h>
#include <z_kxx/util/ani.h>

namespace e
{
	class Ring;
	class BossA : public Boss
	{
	public:
		BossA(Stage * _stage, int _index_in_stage);
		~BossA();
		void BeginFight() override;
	};
}
