
// #include "../config.h"
#include <z_kxx/sprite/simple_sprite.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	// E_DEFINE_MEMORY_POOL(GeneralAura);

	SimpleSprite::SimpleSprite(const stringa & _name)
	{
		tex = kxxwin->LoadTex(_name);
		if(tex)
		{
			hsz.x = tex->W() * 0.5f;
			hsz.y = tex->H() * 0.5f;
		}
	}


	SimpleSprite::~SimpleSprite()
	{
		//if(tex)
		//{
		//	tex->Release();
		//}
	}

	void SimpleSprite::Render()
	{
		Sprite::Render();
	}

}
