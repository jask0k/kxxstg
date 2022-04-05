
#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class Stage1 : public Stage
	{
	public:
		Stage1();
		~Stage1();
		void OnCreateBoss(int _index) override;
		void OnBossFight(int _index) override;
	};
}
