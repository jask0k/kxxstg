
// #include "../config.h"
#include <z_kxx/shot/uniform_shot.h>

namespace e
{
	UniformShot::UniformShot(TexRef _tex)
	{
		tex = _tex;
	}


	UniformShot::~UniformShot()
	{
	}


	bool UniformShot::Step()
	{
		pos.x+= speed.x;
		pos.y+= speed.y;
		return !DisappearTest();
	}


}
