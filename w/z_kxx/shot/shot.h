
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	class Shot : public Sprite
	{
	public:
		bool dead;
		float power;
		uint32 unavailableTimer;
		Vector2 collisionFrac; 
		bool isEllipseCollision;
		bool fragile;
		Shot();
		virtual bool Collide(const Vector2 & _v, float _r);
		virtual void OnHit(const Vector2 & _pt);
		virtual void OnCrash(const Vector2 & _pt);
		virtual void DropItem();
		virtual void RenderDebug();
		bool DisappearTest();
		bool Step() override;
		void OnMasterDestroyed() override;
	};

	class EnemyShot : public Shot
	{
	};

	class PlayerShot : public Shot
	{
	public:
		bool puncture;
		bool fire_sputter;
		void ActivateSputter();
		PlayerShot();
	};

	typedef List<PlayerShot*> PlayerShotList;
	typedef List<EnemyShot*> EnemyShotList;
}

