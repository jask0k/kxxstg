#ifndef NB_CORE_SET_H
#define NB_CORE_SET_H

#include <string.h>
#include <stdlib.h>
#include <nbug/core/debug.h>
#include <new>
//#include <nbug/core/nd_memory.h>

namespace e
{
	template<typename T>
	class Set
	{
	public:
		struct Node;
		typedef Node * NodePtr;
		struct Node
		{
			T v;
			int top;
			NodePtr link[1];
		};
	private:
		static const int MaxTrack = 12;

		Node * new_Node(const T & _v)
		{
			int i;
			for(i = 0; i < MaxTrack-1 && (rand() & 0x01); i++)
			{}
			Node * p = (Node*) nd_malloc(sizeof(Node) + sizeof(NodePtr) * (i) );
			new((void*)&p->v) T(_v);
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
			(&p->v)->~T();
			nd_free(p);
		}
	public:
		class iterator
		{
			friend class Set<T>;
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

			T * operator->()
			{
				E_BASIC_ASSERT(p);
				return &p->v;
			}

			T & operator*()
			{
				E_BASIC_ASSERT(p);
				return p->v;
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
		Set()
		{
			head_node = new_Node();
		}

		~Set()
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

		Set(const Set & _r)
		{
			head_node = new Node();
			Copy(_r);
		}

		const Set & operator=(const Set & _r)
		{
			Copy(_r);
			return *this;
		}

		void Copy(const Set & _r)
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

	public:
		iterator insert(const T & _v)
		{
			Node * p = new_Node(_v);
			int top = p->top;
			Node * pCur = head_node;
			int i = head_node->top;

			while(i>top)
			{
				Node * & l = pCur->link[i];

				if(l!=0)
				{
					if(l->v < _v)
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

				if(l==0 || _v < l->v )
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

		T & operator[](const T & _v)
		{
			Node * p = find(_v).p;
			if(p)
			{
				return p->v;
			}
			else
			{
				return insert(_v, T())->v;
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

		iterator find(const T & _k)
		{
			Node * pCur = head_node;
			int i = head_node->top;
			while(i>=0)
			{
				Node * & l = pCur->link[i];

				if(l!=0)
				{
					if(l->v < _k)
					{
						pCur = l;
						continue;
					}
					else if(l->v == _k)
					{
						return iterator(l);
					}
				}
				i--;
			}

			return iterator(0);
		}
		void erase(const T & _k)
		{
			iterator it = find(_k);
			if(it.p)
			{
				erase(it);
			}
		}
		void erase(iterator _it)
		{
			E_BASIC_ASSERT(_it != end());

			Node * idx = _it.p;
			const T & v = idx->v;

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
					else if(l->v < v || l->v == v)
					{
						pCur = l;
						continue;
					}
				}
				i--;
			}

			delete_Node(idx);
		}

		bool empty() const
		{
			return head_node->link[0] == 0;
		}

		void swap(Set & _r)
		{
			Node * tmp_head_node = this->head_node;
			this->head_node = _r.head_node;
			_r.head_node = tmp_head_node;
		}
	};
}
#endif
