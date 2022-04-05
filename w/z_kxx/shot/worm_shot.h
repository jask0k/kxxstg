
#pragma once

#include <z_kxx/shot/shot.h>

namespace e
{
	class Worm
	{
	public:
		Array<Vector2> vertices;
		Array<Vector2> spine;
		size_t sectionCount;
		size_t usedSectionCount;
		float halfWidth;
		float angle;
		void _Shift(const Vector2 & _c, const Vector2 & _l, const Vector2 & _r);
		void Init(size_t _sectionCount, float _w, const Vector2 & _pos, float _angle);
		Worm();
		~Worm();
		void Render(Graphics * _g);
		void RenderDebug(Graphics * _g);
		void Step(float _dx, float _dy, float _da);
		bool Collide(const Vector2 & _v, float _r);
		const Vector2 & LastPos()
		{ return spine[sectionCount-1]; }
		void DropItem(int _skip, int _hue);
		bool Disappear();
	};

	class GuideWormShot : public EnemyShot
	{
		static const int wormStepFrameCount = 1;
		uint wormStepTimer; 
		uint initialTimerMax;
		uint initialTimer;
		uint durationTimer;	
		float wormSectionLen;
		//float wormAngle;
		Worm worm;
	public:
		GuideWormShot(TexRef _tex, float _w, float _len, const Vector2 & _pt, float _speed, float _angle, float _initialSecond, float _durationSecond);
		bool Step() override;
		void Render() override;
		void DropItem() override;
		void OnCrash(const Vector2 & _pt) override;
		bool Collide(const Vector2 & _v, float _r) override;
		void RenderDebug() override;
	};
}


