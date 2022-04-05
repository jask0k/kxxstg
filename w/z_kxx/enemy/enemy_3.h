
#pragma once

#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani5.h>

namespace e
{
	class Enemy3 : public Enemy
	{
		Ani5 ani5;
		const uint type;
		const uint index;
		const uint total;
		int state;
		uint timer;
		Vector2 vel;
		uint shot_timer;
		float vel_f;
		float a;
		float da;
		uint timer2;
	public:
		Enemy3(uint _type, uint _index, uint _total);
		~Enemy3();
		bool Step() override;
	};


	class Enemy3Maker : public RepeatEnemyMaker
	{
		uint type;
	public:
		Enemy3Maker(uint _type);
		Enemy * OnMakeOne() override;
	};
}

