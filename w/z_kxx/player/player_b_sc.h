
#pragma once
#include <z_kxx/shot/shot.h>
namespace e
{
	class PlayerB;
	class MasterSpark : public PlayerShot
	{
		PlayerB * player;
		uint timer;
		float da;
		float w;
	public:
		MasterSpark(PlayerB * _player);
		~MasterSpark();
		bool Step() override;
		void Render() override;
		bool Collide(const Vector2 & _v, float _r) override;
	};

}
