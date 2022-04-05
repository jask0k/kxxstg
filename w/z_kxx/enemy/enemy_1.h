
#pragma once
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani.h>

namespace e
{
	class Enemy1 : public Enemy
	{
		Ani ani;
		const bool leftToRight; // sub type
		int state;
		float dy;
		uint stateTimer;
	public:
		Enemy1(bool _leftToRight, float _x, float _y);
		~Enemy1();
		bool Step() override;
	};

	class Enemy1Maker : public RepeatEnemyMaker
	{
		bool left;
	public:
		Enemy1Maker();
		Enemy * OnMakeOne() override;
	};
}
