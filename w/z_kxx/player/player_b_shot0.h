
#pragma once
#include <z_kxx/shot/shot.h>

namespace e
{
	class PlayerB;
	class PlayerBShot0 : public PlayerShot
	{
		PlayerB * player;
	public:
		Vector2 speed;
		PlayerBShot0(PlayerB * _player);
		~PlayerBShot0();
		bool Step() override;
		void OnCrash(const Vector2 & _pt) override; 
	};
	//typedef List<PlayerBShot0*> UniformShotList;
}
