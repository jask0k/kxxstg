
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class PlayerA;
	class PlayerAShot0 : public PlayerShot
	{
		PlayerA * player;
		uint32 transparent_timer;
	public:
		Vector2 speed;
		PlayerAShot0(PlayerA * _player);
		~PlayerAShot0();
		bool Step() override;
		void OnCrash(const Vector2 & _pt) override; 
	};
	//typedef List<PlayerAShot0*> UniformShotList;
}


