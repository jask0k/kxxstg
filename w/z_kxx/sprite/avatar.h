
#pragma once
#include <z_kxx/sprite/sprite.h>

namespace e
{
//	class TexRef;
	class Avatar : public Sprite
	{
		uint timer;
		Vector2 speed;
	public:
		bool Step() override;
		void CreateBoss(TexRef _tex);
		void CreatePlayer(TexRef _tex);
	};
}


