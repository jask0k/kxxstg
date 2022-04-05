#pragma once

#include <z_kxx/stage/stage.h>

namespace e
{
	class BossASC1BG : public StageBG
	{
	public:
		float w;
		float h;
		float angle;
		float angle_delta;
		float alpha;
		float alpha_delta;
		int state;
		uint32 timer;
		TexRef tex;
		BossASC1BG();
		~BossASC1BG();
		bool Step() override;
		void Render() override;
		void FadeOut() override;
	};
}
