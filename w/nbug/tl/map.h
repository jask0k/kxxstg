#ifndef NB_CORE_MAP_H
#define NB_CORE_MAP_H

#include <string.h>
#include <stdlib.h>
#include <new>
#include <nbug/core/debug.h>
//#include <nbug/core/nd_memory.h>

namespace e
{
	// i7 920 实测效率: 
	// Map<string,int>, 平均关键字长16.6字符. 100个元素约800万次/秒, 10000个元素约450万次/秒
	// Map<int, int> 1000元素约1400万次/秒, 10000个元素约800万次/秒
	// 比vc2010的std::map慢一倍, 有优化空间?
	template<typename K, typename V>
	class Map
	{
	public:
		struct Node;
		typedef Node * NodePtr;
		struct Node
		{
			K first;
			V second;
			int top;
			NodePtr link[1];
		};
	private:
		static const int MaxTrack = 12;

		Node * new_Node(const K & _k, const V & _v)
		{
			int i;
			for(i = 0; i < MaxTrack-1 && (rand() & 0x01); i++)
			{}
			Node * p = (Node*) nd_malloc(sizeof(Node) + sizeof(NodePtr) * (i) );
			new((void*)&p->first) K(_k);
			new((void*)&p->second) V(_v);
			p->top = i;
			return p;
		}

		Node * new_Node()
		{
			Node * p = (Node*) nd_malloc(sizeof(Node) + sizeof(NodePtr) * (MaxTrack-1));
			p->top = MaxTrack-1;
			memset(&p->link, 0, sizeof(NodePtr) * (MaxTrack));
			return p;
		}

		void delete_Node(Node * p)
		{
			E_BASIC_ASSERT(p != head_node);
			(&p->first)->~K();
			(&p->second)->~V();
			nd_free(p);
		}
	public:
		class iterator
		{
			friend class Map<K, V>;
			Node * p;
		public:
			iterator() : p(0)
			{}
			iterator(Node * _p) : p(_p)
			{ }
			iterator(const iterator & _r) : p(_r.p)
			{ }
			void operator++()
			{
				E_BASIC_ASSERT(p);
				p = p->link[0];
			}

			Node * operator->()
			{
				E_BASIC_ASSERT(p);
				return p;
			}

			Node & operator*()
			{
				E_BASIC_ASSERT(p);
				return *p;
			}

			bool operator==(const iterator & _r)
			{
				return p == _r.p;
			}
			bool operator!=(const iterator & _r)
			{
				return p != _r.p;
			}
		};
	private:

		Node * head_node;

	public:
		Map()
		{
			head_node = new_Node();
		}

		~Map()
		{
			Node * pCur = head_node->link[0];
			while(pCur)
			{
				Node * p = pCur;
				pCur = pCur->link[0];
				delete_Node(p);
			}
			nd_free(head_node);
		}

		Map(const Map & _r)
		{
			head_node = new Node();
			Copy(_r);
		}

		const Map & operator=(const Map & _r)
		{
			Copy(_r);
			return *this;
		}

		void Copy(const Map & _r)
		{
			clear();
			if(this != & _r)
			{
				Node * p = _r.head_node;
				if(p)
				{
					p = p->link[0];
				}
				while(p)
				{
					insert(p->data);
					p = p->link[0];
				}
			}
		}

		iterator begin()
		{
			return iterator(head_node->link[0]);
		}

		iterator end()
		{
			return iterator(0);
		}

	private:
		iterator insert(const K & _k, const V & _v)
		{
			Node * p = new_Node(_k, _v);
			int top = p->top;
			Node * pCur = head_node;
			int i = head_node->top;

			while(i>top)
			{
				Node * & l = pCur->link[i];

				if(l!=0)
				{
					if(l->first < _k)
					{
						pCur = l;
						continue;
					}
				}
				i--;
			}

			while(i>=0)
			{
				Node * & l = pCur->link[i];

				if(l==0 || _k < l->first )
				{
					p->link[i] = l;
					l = p;
					i--;
				}
				else
				{
					pCur = l;
				}
			}

			return iterator(p);
		}
	public:
		V & operator[](const K & _first)
		{
			Node * p = find(_first).p;
			if(p)
			{
				return p->second;
			}
			else
			{
				return insert(_first, V())->second;
			}
		}

		void clear()
		{
			Node * pCur = head_node->link[0];
			while(pCur)
			{
				Node * p = pCur;
				pCur = pCur->link[0];
				delete_Node(p);
			}
			memset(&head_node->link, 0, sizeof(NodePtr) * (MaxTrack));
		}
		
		iterator find(const K & _first)
		{
			Node * pCur = head_node;
			int i = head_node->top;
			while(i>=0)
			{
				Node * & l = pCur->link[i];

				if(l!=0)
				{
					if(l->first < _first)
					{
						pCur = l;
						continue;
					}
					else if(l->first == _first)
					{
						return iterator(l);
					}
				}
				i--;
			}

			return iterator(0);
		}
		void erase(const K & _k)
		{
			iterator it = find(_k);
			if(it.p)
			{
				erase(it);
			}
		}
		iterator erase(iterator _it)
		{
			E_BASIC_ASSERT(_it != end());

			Node * idx = _it.p;
			const K & first = idx->first;
			Node * next = idx->link[0];
			Node * pCur = head_node;
			int i = head_node->top;
			while(i>=0)
			{
				Node * & l = pCur->link[i];

				if(l!=0)
				{
					if(l == idx)
					{
						l = idx->link[i];
					}
					else if(l->first < first || l->first == first)
					{
						pCur = l;
						continue;
					}
				}
				i--;
			}

			delete_Node(idx);
			return next;
		}

		bool empty() const
		{
			return head_node->link[0] == 0;
		}

		void swap(Map & _r)
		{
			Node * tmp_head_node = this->head_node;
			this->head_node = _r.head_node;
			_r.head_node = tmp_head_node;
		}
	};
}
#endif
