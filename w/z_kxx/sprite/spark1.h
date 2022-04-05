
#pragma once
#include <z_kxx/sprite/sprite.h>
#include <z_kxx/util/cd_counter.h>
namespace e
{
//	class TexRef;
	class Spark1 : public Sprite
	{
		float angle0, angleDelta;
		float radius0, radiusDelta;
		float alpha0, alphaDelta;
		CDCounter counter;
	public:
		Vector2 speed;
		Spark1(TexRef _tex, float _second, float _angle0, float _angle1, float _radius0, float _radius1, float _alpha0, float _alpha1);
		~Spark1();
		void Render() override;
		bool Step() override;
	};
}


