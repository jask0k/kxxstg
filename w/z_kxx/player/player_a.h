
#pragma once
#include <z_kxx/player/player.h>
#include <z_kxx/util/ani.h>
namespace e
{
	class PlayerA : public Player
	{
	public:
		TexRef yyyTex;
		TexRef shot0;
		TexRef shot1;
		Ani shot0Spark;
		Ani shot1Spark;
		PlayerA();
		~PlayerA();
		void Fire() override;
		void CastSpellCard() override;
		void RenderPets() override;
		bool Step() override;
//		float GetSpellCardDamage(const Vector2 & _pt, float _r) override;
	};
}
