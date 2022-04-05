
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class GeneralAura : public Sprite
	{
	public:
		GeneralAura(TexRef _tex);
		~GeneralAura();
		void Render() override;
		bool Step() override;
	};
}

