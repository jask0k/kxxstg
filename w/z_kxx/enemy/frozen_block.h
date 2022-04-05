
#pragma once
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani.h>

namespace e
{
	class FrozenBlock : public Enemy
	{
		PlayerShot * playerShot;
		uint32 timer;
		uint32 active_time;
		uint32 init_timer;
		RGBA init_clr;
		Vector2 pos_offset;
	public:
		bool active;
		FrozenBlock(PlayerShot * _playerShot);
		~FrozenBlock();
		bool Step() override;
		void Render() override;
		//void Damage(float _damage, bool _ignore_defence) override;
	};
}
