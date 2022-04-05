
// #include "../config.h"
#include <z_kxx/util/render_list.h>

namespace e
{
	RenderList::RenderList()
	{
	}


	RenderList::~RenderList()
	{
	}

	void RenderList::Render(uint _from, uint _to)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(_from >= 0 && _to <= RL_ALL_TOP);
		for(uint i=_from; i<=_to; i++)
		{
			Sprite * p = layer[i].head;
			while(p)
			{
				p->Render();
				p = p->rlist_next;
			}
		}
	}

	void RenderList::clear()
	{
		// NB_PROFILE_INCLUDE;
		for(uint i=0; i<MAX_LAYER; i++)
		{
			layer[i].head = 0;
			layer[i].tail = 0;
		}
	}

	uint RenderList::GetSize() const
	{
		// NB_PROFILE_INCLUDE;
		int ret = 0;
		for(uint i=0; i<MAX_LAYER; i++)
		{
			Sprite * p = layer[i].head;
			while(p)
			{
				ret++;
				p = p->rlist_next;
			}
		}
		return ret;
	}
}
