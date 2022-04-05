
#pragma once

#include <z_kxx/boss/boss.h>

namespace e
{
	class Ring;
	class BossB : public Boss
	{
	public:
		BossB(Stage * _stage, int _index_in_stage);
		~BossB();
	};
}
