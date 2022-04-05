
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class Laser : public EnemyShot
	{
	//	Vector2 offset;
		float r0;
		float angle_delta;
		uint32 timer;
		uint32 timer_state2;
		int state;
		float scale_delta;
	public:
		Laser(Sprite * _master, TexRef _tex, float _w, float _a, float _da, float _r0, float _r, uint32 _t);
		bool Step() override;
		void OnHit(const Vector2 & _pt) override;
		void OnCrash(const Vector2 & _pt) override;
		void DropItem() override;
		void Render() override;
		bool Collide(const Vector2 & _v, float _r) override;
	private:
		void SetAndle(float _a);
	};
}


