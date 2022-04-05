
#pragma once

#include <z_kxx/shot/shot.h>
#include <z_kxx/util/cd_counter.h>
namespace e
{
	class LapillusShot : public EnemyShot
	{
		TexRef smokeTex;
		CDCounter fadeIn;
		uint32 t1;
	public:
		Vector2 speed;
		Vector2 gravity;
		LapillusShot();
		~LapillusShot();
		void Render() override;
		bool Step() override;
	};
}


