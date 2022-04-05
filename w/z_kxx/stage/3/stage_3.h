
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage3 : public Stage
	{
	public:
		Stage3();
		~Stage3();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
