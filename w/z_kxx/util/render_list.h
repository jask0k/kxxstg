
#pragma once

#include <z_kxx/sprite/sprite.h>

namespace e
{
	enum
	{
		RL_UI_BACK_0, // background image
		RL_UI_BACK_1, // background object

		RL_SCENE_0, // scene background
		RL_SCENE_1, // scene background

		RL_GAME_BOTTOM,
		RL_GAME_LOW_AURA,
		RL_GAME_ENEMY_AURA,
		RL_GAME_PLAYER_SHOT,
		RL_GAME_SPARK,
		RL_GAME_DROP,
		RL_GAME_PLAYER_AURA,
		RL_GAME_PLAYER,
		RL_GAME_ENEMY,
		RL_GAME_BOSS = RL_GAME_ENEMY,
		RL_GAME_PLAYER_PET,
		RL_GAME_ENEMY_SHOT,
		RL_GAME_TEXT,
		RL_GAME_TOP,
		
		RL_UI_FRONT_0, // front overlay image
		RL_UI_FRONT_1, // front sprite
		RL_UI_FRONT_2, // text

		RL_ALL_TOP = RL_UI_FRONT_2,

	};
	class Sprite;
	class RenderList
	{
		class Layer
		{
		public:
			Sprite * head;
			Sprite * tail;

			Layer() 
				: head(0)
				, tail(0)
			{}

			void push_back(Sprite * _p)
			{
				_p->rlist_prev = tail;
				_p->rlist_next = 0;
				if(tail)
				{
					tail->rlist_next = _p;
					tail = _p;
				}
				else
				{
					tail = _p;
					head = _p;
				}
			}

		};
		void Remove(Sprite * _p)
		{
			E_ASSERT(_p);
			//AnimateSprite * ret = _p;
			if(_p->rlist_prev)
			{
				_p->rlist_prev->rlist_next = _p->rlist_next;
			}
			else
			{
				Layer & l = layer[_p->rlist_layer];
				l.head = (_p->rlist_next);
				if(l.head == 0)
				{
					l.tail = 0;
				}
			}

			if(_p->rlist_next)
			{
				_p->rlist_next->rlist_prev = _p->rlist_prev;
			}
			else
			{
				Layer & l = layer[_p->rlist_layer];
				l.tail = (_p->rlist_prev);
				if(l.tail == 0)
				{
					l.head = 0;
				}
			}
			_p->rlist_layer = -1;
		}

		void Add(Sprite * _p, uint _layer)
		{
			E_ASSERT(_p->rlist_layer == -1);
			E_ASSERT(_p);
			E_ASSERT(_layer < MAX_LAYER);
			_p->rlist_layer = _layer;
			layer[_layer].push_back(_p);
		}

		friend class Sprite;
	public:
		static const int MAX_LAYER = RL_ALL_TOP+1;
		RenderList();
		~RenderList();


		void Render(uint _from, uint _to);
		void RenderAll()
		{
			Render(0, MAX_LAYER-1);
		}
		void clear();
		uint GetSize() const;
		bool IsLayerEmpty(int _layer) const
		{ return layer[_layer].head == 0; }
	private:
		Layer layer[MAX_LAYER];
	};

}
