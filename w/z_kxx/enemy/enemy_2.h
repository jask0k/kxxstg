
#pragma once

#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani5.h>

namespace e
{
	class Enemy2 : public Enemy
	{
		Ani5 ani5;
		const uint type;
		const uint index;
		const uint total;
		int state;
		uint timer;
		Vector2 acc;
		Vector2 vel;
	public:
		Enemy2(uint _type, uint _index, uint _total);
		~Enemy2();
		bool Step() override;
	};


	class Enemy2Maker : public RepeatEnemyMaker
	{
		uint type;
	public:
		Enemy2Maker(uint _type);
		Enemy * OnMakeOne() override;
	};
}

