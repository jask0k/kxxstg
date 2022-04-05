
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage2 : public Stage
	{
	public:
		Stage2();
		~Stage2();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
