
#pragma once

#include <z_kxx/enemy/enemy.h>
#include <z_kxx/util/ani5.h>

namespace e
{
	class Enemy0 : public Enemy
	{
		Ani5 ani5;
		const int type; // sub type
		Vector2 speed;
		enum State
		{
			sStand, //
			sForward, //
			sShot, //
			sEscape, //
			_STATE_MAX,
		};
		State state;
		uint stateTimer;
		void SetState(State _s);
		uint prob[_STATE_MAX];
		//uint totalProb;
	public:
		Enemy0(int _type, float _x, float _y);
		~Enemy0();
		bool Step() override;
	};


	class Enemy0Maker : public RepeatEnemyMaker
	{
	public:
		Enemy0Maker();
		Enemy * OnMakeOne() override;
	};
}

