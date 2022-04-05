
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class UniformShot : public EnemyShot
	{
	public:
		Vector2 speed;
		UniformShot(TexRef _tex);
		~UniformShot();
		bool Step() override;
	};
	//typedef List<UniformShot*> UniformShotList;
}


