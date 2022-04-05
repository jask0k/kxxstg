
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage6 : public Stage
	{
	public:
		Stage6();
		~Stage6();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
