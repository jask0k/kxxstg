
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage4 : public Stage
	{
	public:
		Stage4();
		~Stage4();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
