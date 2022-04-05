
#pragma once

#include <z_kxx/boss/boss.h>

namespace e
{
	class Ring;
	class BossAMid : public Boss
	{
	public:
		//Galaxy * gathering;
		//bool showAura0;
		//bool showAura1;
		BossAMid(Stage * _stage, int _index_in_stage);
		~BossAMid();
		void BeginFight() override;
	};
}
