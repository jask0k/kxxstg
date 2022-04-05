
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class PlayerA;
	class Enemy;
	class PlayerAShot1 : public PlayerShot
	{
		PlayerA * player;
		uint32 transparent_timer;
		uint32 guide_timer;
		Vector2 vel;
	public:
		PlayerAShot1(PlayerA * _player, const Vector2 & _pos, float _ang);
		~PlayerAShot1();
		void OnCrash(const Vector2 & _pt) override; // hit target
		bool Step() override;
	};
}

