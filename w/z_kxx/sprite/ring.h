
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	// slow
	bool CalcRingVertices(Vector2 _buf[], size_t _size, float _innerRadius, float _outerRadous);

	class Ring : public Sprite
	{
		//TexRef tex;
		Vector2 * vertices;
		size_t size;
		float innerRadius;
		float outerRadous;
		float angleDelta;
	public:
		Ring(TexRef _tex, float _innerRadius, float _outerRadous, float _angleDelta, uint _sectionCount = 64);
		~Ring();
		void SetRadius(float _innerRadius, float _outerRadous);
		void SetScale(float _scale);
		void Render();
		bool Step() override;
	};
}
