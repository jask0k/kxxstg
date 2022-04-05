
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage7 : public Stage
	{
	public:
		Stage7();
		~Stage7();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
