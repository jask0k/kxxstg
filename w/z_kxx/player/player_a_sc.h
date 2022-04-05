
#pragma once
#include <z_kxx/shot/shot.h>
namespace e
{
	class PlayerAYYYSC : public PlayerShot
	{
		float scale_delta;
		float speed_y;
		float alpha_delta;
		float angle_delta;
		uint timer;
	public:
		PlayerAYYYSC(const Vector2 & _pt, TexRef _tex);
		~PlayerAYYYSC();
		bool Step() override;
		void OnCrash(const Vector2 & _pt) override;
	};
}

