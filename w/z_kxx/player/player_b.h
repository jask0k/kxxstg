
#pragma once

#include <z_kxx/player/player.h>
#include <z_kxx/util/ani.h>
namespace e
{
	// Kirisame PlayerB.
	class PlayerB : public Player
	{
	public:
		TexRef texMasterSpark;
		TexRef shot0;
		TexRef laser;
		Ani shot0Spark;
		PlayerB();
		~PlayerB();
		void Fire() override;
		void CastSpellCard() override;
		void RenderPets() override;
		bool Step() override;
		void Render() override;
	};
}

