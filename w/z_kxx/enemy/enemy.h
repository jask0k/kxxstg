
#pragma once
#include <z_kxx/sprite/unit.h>
#include <z_kxx/globals.h>
#include <z_kxx/shot/enemy_shots.h>
#include <z_kxx/drop/drop_def.h>

namespace e
{
	class Stage;
	class EnemyGroup;
	class Enemy : public Unit
	{
	public:
		float init_life;
		float life;
		float def;
		float dr;
		bool ethereal;
		bool IsEthereal() const
		{ return ethereal; }
		void OnMasterDestroyed() override;
		EnemyGroup * group;
		Sprite * aura;
		DropDef drops;
		Enemy();
		~Enemy();
		virtual void Damage(float _damage, bool _ignore_defence);
		bool Step() override;
		virtual bool OnLifeEmpty();
		void RenderDebug() override;
		float GetCollisionRadius() const
		{ return hsz.x; }
	};
	typedef List<Enemy*> EnemyList;


	class EnemyGroup
	{
	protected:
		virtual ~EnemyGroup();
		Vector2 lastAddPos;
		Vector2 lastRemovePos;
		bool attached;
		int ref_count;
		int total;
		int kill;
		virtual void OnAdd(Enemy * _p);
		virtual void OnRemove(Enemy * _p);
	public:
		EnemyGroup();
		void Add(Enemy * _p);
		void Remove(Enemy * _p, bool _kill);

		// enew EnemyGroup has extra Ref count, we must call
		//	Detach() after all member has been created.
		void Detach();
	};

	class LastDropGroup : public EnemyGroup
	{
	protected:
		void OnRemove(Enemy * _p) override;
	public:
		DropDef drops;
	};


	class EnemyMaker : public Sprite
	{
	};

	class EnemyGroup;
	class RepeatEnemyMaker : public EnemyMaker
	{
	public:
		RepeatEnemyMaker(uint _count, float _span, float _initDelay = 0);
		uint GetCurrent() const
		{ return int_count - count - 1; }
		uint GetTotal() const
		{ return int_count; }
	protected:
		~RepeatEnemyMaker();
		const uint int_count;
		uint count;
		uint span;
		uint timer;
		EnemyGroup * group;
		virtual Enemy * OnMakeOne() = 0;
		bool Step() override;
	};

	void add_enemy(int _type, int _param);
}
