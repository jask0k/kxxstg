
// #include "../config.h"
#include <z_kxx/enemy/enemy.h>
#include <z_kxx/stage/stage.h>
#include <z_kxx/main/kxxwin.h>
#include <z_kxx/shot/enemy_shots.h>

namespace e
{
	// E_DEFINE_MEMORY_POOL(Enemy);

	Enemy::Enemy()
	{
		init_life = 0;
		life = 0;
		aura = 0;
		group = 0;
		master = 0;
		pets = 0;
		ethereal = false;
		dr = kxxwin->state.level * 0.1f;
		def = 0;
	}

	Enemy::~Enemy()
	{
		if(group)
		{
			group->Remove(this, life <= 0);
		}
		delete aura;
	}

	bool Enemy::Step() 
	{
		Unit::Step();

		E_ASSERT(life <= init_life);

		if(aura)
		{
			aura->pos = this->pos;
		}

		return !IsOutOfGameArea(pos, 42);
	}

	//virtual void RenderAura();
	void DropDef::Set(int _slotIndex, uint _type, uint _min, int _max)
	{
		E_ASSERT(_slotIndex >= 0 && _slotIndex < 3);
		slot[_slotIndex].type = _type;
		slot[_slotIndex].random_min = _min;
		slot[_slotIndex].random_max = _max < 0 ? _min : _max;
	}

	bool Enemy::OnLifeEmpty()
	{
		kxxwin->AddEnemyExplosion(this);
		kxxwin->AddEnemyDrop(drops, pos);
		E_ASSERT(init_life > 0);
		Enemy * p = dynamic_cast<Enemy*>(master);
		if(p)
		{
			p->Damage(init_life * 0.2f, true);
		}

		kxxwin->player->AddPoint(pos, POINT_GAIN_KILL_ENEMY0);
		kxxwin->PlayEnemyDeadSE(pos);
		return true;
	}


	void Enemy::Damage(float _damage, bool _ignore_defence)
	{
		if(!_ignore_defence)
		{
			_damage = (_damage - def) * (1.0f - this->dr);
		}

		if(_damage > 0)
		{
			life-= _damage;
			Enemy * p = dynamic_cast<Enemy*>(master);
			if(p)
			{
				p->Damage(_damage * 0.3f, true);
			}

			kxxwin->player->AddPoint(pos, _damage * POINT_GAIN_HIT_ENEMY);
		}
	}

	void Enemy::OnMasterDestroyed()
	{
		life = 0;
	}



	void Enemy::RenderDebug()
	{
#ifdef NB_DEBUG
		graphics->SetFont(kxxwin->debugFont);
		string a(life, 1);
		graphics->SetColor(1, 0.5, 0.5, 1);
		graphics->DrawString(pos.x - 10, pos.y - 15, a);
		string b = L"+" + string(def, 1) + " *" + string(dr, 1);
		graphics->SetColor(0.5, 0.5, 1, 1);
		graphics->DrawString(pos.x - 10, pos.y, b);
		if(this->ethereal)
		{
			graphics->SetColor(0.5, 1, 0.5, 1);
			graphics->DrawString(pos.x - 10, pos.y + 15, "ETH");
		}
#endif
	}


	EnemyGroup::EnemyGroup()
	{
		//E_TRACE_LINE("[kx] EnemyGroup::EnemyGroup()");
		attached = true;
		ref_count = 1;
		total = 0;
		kill = 0;
	}

	EnemyGroup::~EnemyGroup()
	{
		//E_TRACE_LINE("[kx] EnemyGroup::~EnemyGroup()");
	}

	void EnemyGroup::Add(Enemy * _p)
	{
		E_ASSERT(_p->group == 0);
		_p->group = this;
		ref_count++;
		total++;
		lastAddPos = _p->pos;
		OnAdd(_p);
	}

	void EnemyGroup::Remove(Enemy * _p, bool _kill)
	{
		lastRemovePos = _p->pos;
		ref_count--;
		if(_kill)
		{
			kill++;
		}
		OnRemove(_p);
		if(ref_count <= 0)
		{
			delete this;
		}
	}

	void EnemyGroup::OnAdd(Enemy * _p)
	{
	}

	void EnemyGroup::OnRemove(Enemy * _p)
	{
	}

	void EnemyGroup::Detach()
	{
		if(attached)
		{
			attached = false;
			ref_count--;
			if(ref_count <= 0)
			{
				delete this;
			}
		}
	}

	void LastDropGroup::OnRemove(Enemy * _p)
	{
		if(ref_count == 0 && kill == total)
		{
			kxxwin->AddEnemyDrop(drops, lastRemovePos);
		}
	}

	RepeatEnemyMaker::RepeatEnemyMaker(uint _count, float _span, float _initDelay)
		: int_count(_count)
	{
		group = 0;
		count = _count;
		int span1 = S2F(_span);
		if(span1 < 1)
		{
			span1 = 1;
		}
		span = span1;
		int init = S2F(_initDelay);
		if(init < 1)
		{
			init = 1;
		}
		timer = init;
	}

	RepeatEnemyMaker::~RepeatEnemyMaker()
	{
		if(group)
		{
			group->Detach();
		}
	}

	bool RepeatEnemyMaker::Step()
	{
		if(count == 0)
		{
			if(group)
			{
				group->Detach();
			}
			return false;
		}

		E_ASSERT(timer);
		timer--;
		if(timer == 0)
		{
			count--;
			timer = span;
			Enemy * enemy = OnMakeOne();
			if(enemy)
			{
				if(group)
				{
					group->Add(enemy);
				}
				kxxwin->AddEnemyToList(enemy);
			}
		}

		if(count == 0)
		{
			if(group)
			{
				group->Detach();
				group = 0;
			}
			return false;
		}
		else
		{
			return true;
		}
	}


}
