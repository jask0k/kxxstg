
#pragma once
#include <z_kxx/shot/shot.h>
namespace e
{
	class PlayerBShot1 : public PlayerShot
	{
		TexRef petTex;
		uint timer;
		float y0;
		float y1;
		float y2;
	public:
		PlayerBShot1(const Vector2 & _pt, TexRef _tex, TexRef _petTex, float _power);
		~PlayerBShot1();
		bool Step() override;
		bool Collide(const Vector2 & _v, float _r) override;
		void Render() override;
	};
}

