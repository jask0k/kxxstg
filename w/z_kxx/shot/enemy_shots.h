
#pragma once
#include <nbug/math/math.h>
namespace e
{
	enum
	{
		ES_SNIPE_0,
		ES_KEDAMA_SCATTER,
		ES_KEDAMA_SNIPE,
		ES_LING_SCATTER,
		ES_LING_SNIPE,
		ES_OPPOSITE_SPIRE,
		ES_LINE_MESS,
		ES_SNIPE_ARRAY,
		ES_4ROSE,
		ES_THROW,
		ES_LASER_FAN, //
		ES_LAPILLUS, //
		ES_VOLCANO, //
		ES_DS2_2, // a danmaku from DS 2-2
		ES_WORM, //
		ES_SHOTGUN, //
		ES_COMET,
	//	ES_THROW_EXPLODE,
	};

	void AddEnemyShot(Vector2 & _pt, int _type);
}
