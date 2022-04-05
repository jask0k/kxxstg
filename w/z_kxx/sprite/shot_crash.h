
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class ShotCrash : public Sprite
	{
	public:
		uint32 timer0;
		uint32 timer1;
		float alpha;
		float alpha_delta;
		float w;
		VCT * v;
		size_t size;
		ShotCrash(TexRef & _tex);
		~ShotCrash();
		bool Step() override;
		void Render() override;
	private:
		void ChangeW();
	};
}

