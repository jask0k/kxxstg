
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class VolcanoShot : public EnemyShot
	{
		uint32 t0;
		uint32 t1;
		uint32 span;
	public:
		Vector2 speed;
		VolcanoShot(int _span);
		~VolcanoShot();
		void Render() override;
		bool Step() override;
	};

}
