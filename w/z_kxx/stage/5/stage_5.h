
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage5 : public Stage
	{
	public:
		Stage5();
		~Stage5();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
