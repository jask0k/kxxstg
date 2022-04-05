
#pragma once
#include <z_kxx/globals.h>

namespace e
{
	class DropDef
	{
	public:
		struct ENEMY_DROP
		{
			unsigned char type;
			unsigned char random_min;
			unsigned char random_max;
		};
	private:
		ENEMY_DROP slot[3]; //
	public:
		bool allRandomPos;
		DropDef()
		{
			allRandomPos = false;
			slot[0].type = DROP_NONE;
			slot[1].type = DROP_NONE;
			slot[2].type = DROP_NONE;
		}
		void Set(int _slotIndex, uint _type, uint _min = 1, int _max = -1);
		const ENEMY_DROP & Get(int _n) const
		{ return slot[_n]; }

	};
}
