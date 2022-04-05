#include <z_kxx/enemy/enemy.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/enemy/enemy_0.h>
#include <z_kxx/enemy/enemy_1.h>
#include <z_kxx/enemy/enemy_2.h>
#include <z_kxx/enemy/enemy_3.h>
#include <z_kxx/enemy/wild_flower.h>

namespace e
{
	void add_enemy(int _param0, int _param1)
	{
		EnemyMaker * maker = 0;
		switch(_param0)
		{
		case 0:
			maker = enew Enemy0Maker();
			break;
		case 1:
			maker = enew Enemy1Maker();
			break;
		case 2:
			maker = enew Enemy2Maker(_param1);
			break;
		case 3:
			maker = enew Enemy3Maker(_param1);
			break;
		case 4:
		case 5:
		case 6:
			{
				WildFlower * p = enew WildFlower(_param0 - 4, _param1);
				kxxwin->AddEnemyToList(p);
			}
			return;
		}

		if(maker)
		{
			kxxwin->AddLogicActionToList(maker);
		}
		else
		{
			message(L"[kx] (WW) Invalid enemy: param0 = " + string(_param0) + L", param1 = " + string(_param0));
		}
	}
}
