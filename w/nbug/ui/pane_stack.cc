#include <nbug/tl/set.h>
#include <nbug/ui/pane_stack.h>
#include <nbug/ui/pane.h>
#include <nbug/ui/pane_.h>
#include <nbug/gl/rect.h>

namespace e
{
	PaneStack::PaneStack()
	{
		rootPane = 0;
		last_i = -1;
		last_x0 = 1;
		last_x1 = 0;
		last_j = -1;
		last_y0 = 1;
		last_y1 = 0;
		last = 0;
		valid = false;
	}

	void PaneStack::SetRoot(Pane * _root)
	{
		Invalidate();
		rootPane = _root;
	}

	void PaneStack::Invalidate()
	{
		last_i = -1;
		last_x0 = 1;
		last_x1 = 0;
		last_j = -1;
		last_y0 = 1;
		last_y1 = 0;
		last = 0;
		valid = false;
	}


	PaneStack::~PaneStack()
	{
	}

	void PaneStack::FindX(int & _0, int & _1, float _x0, float _x1)
	{
		// NB_PROFILE_INCLUDE;
		if(_x0 > _x1)
		{
			float t = _x0;
			_x0 = _x1;
			_x1 = t;
		}

		_0 = xAxis.size();
		_1 = -1;

		int i;
		for(i = 0; i < (int)xAxis.size(); i++)
		{
			if(xAxis[i].a == _x0)
			{
				_0 = i;
				break;
			}
		}

		for(; i < (int)xAxis.size(); i++)
		{
			if(xAxis[i].b == _x1)
			{
				_1 = i;
				break;
			}
		}
	}

	void PaneStack::FindY(int & _0, int & _1, float _y0, float _y1)
	{
		// NB_PROFILE_INCLUDE;
		if(_y0 > _y1)
		{
			float t = _y0;
			_y0 = _y1;
			_y1 = t;
		}

		_0 = yAxis.size();
		_1 = -1;

		int i;
		for(i = 0; i < (int)yAxis.size(); i++)
		{
			if(yAxis[i].a == _y0)
			{
				_0 = i;
				break;
			}
		}

		for(; i < (int)yAxis.size(); i++)
		{
			if(yAxis[i].b == _y1)
			{
				_1 = i;
				break;
			}
		}
	}

	int PaneStack::FindX(float _x)
	{
		for(int i = 0; i < (int)xAxis.size(); i++)
		{
			if(xAxis[i].a <= _x && xAxis[i].b >= _x)
			{
				return i;
			}
		}
		return -1;
	}

	int PaneStack::FindY(float _y)
	{
		for(int i = 0; i < (int)yAxis.size(); i++)
		{
			if(yAxis[i].a <= _y && yAxis[i].b >= _y)
			{
				return i;
			}
		}
		return -1;
	}

	void PaneStack::RecursiveAddPane(Pane * _p, int _pass, float offx, float offy, Rect * _clip)
	{
		offx+= _p->rect.x;
		offy+= _p->rect.y;
		bool add = (_pass == 0 && !_p->IsPopup() || _pass == 1) && !_p->imp->inStack;
		if(add)
		{
			_p->imp->offset.x = offx;
			_p->imp->offset.y = offy;
			_p->imp->box.x = _p->imp->offset.x;
			_p->imp->box.y = _p->imp->offset.y;
			_p->imp->box.w = _p->rect.w;
			_p->imp->box.h = _p->rect.h;
			if(_clip && !_p->IsPopup())
			{
				_p->imp->box&= *_clip;
			}

			_p->imp->inStack = true;
			stack.push_back(_p);
		}

		for(List<Pane*>::iterator it = _p->imp->children.begin(); it != _p->imp->children.end(); ++it)
		{
			Pane * p = *it;
			if(p->visible && !p->IsPopup())
			{
				RecursiveAddPane(p, 0, offx, offy, &_p->imp->box);
			}
		}

		if(_pass == 1)
		{
			for(List<Pane*>::iterator it = _p->imp->children.begin(); it != _p->imp->children.end(); ++it)
			{
				Pane * p = *it;
				if(p->visible)
				{
					RecursiveAddPane(p, 1, offx, offy, &_p->imp->box);
				}
			}
		}
	}

	void PaneStack::RecursiveMarkToAdd(Pane * _p)
	{
		_p->imp->inStack = false;
		for(List<Pane*>::iterator it = _p->imp->children.begin(); it != _p->imp->children.end(); ++it)
		{
			Pane * p = *it;
			RecursiveMarkToAdd(p);
		}
	}

	void PaneStack::Build()
	{
		// NB_PROFILE_INCLUDE;

		if(valid)
		{
			return;
		}

		stack.clear();
		xAxis.clear();
		yAxis.clear();
		grid.clear();

		if(!rootPane || !rootPane->visible)
		{
			return;
		}
		RecursiveMarkToAdd(rootPane);
		RecursiveAddPane(rootPane, 0, 0, 0, 0);
		RecursiveAddPane(rootPane, 1, 0, 0, 0);

		e::Set<float> xEdges;
		e::Set<float> yEdges;
		const size_t sz = stack.size();
		for(size_t i = 0; i < sz; i++)
		{
			Pane * p = stack[i];
			if(p->visible)
			{
				xEdges.insert(p->imp->box.L());
				xEdges.insert(p->imp->box.R());
				yEdges.insert(p->imp->box.T());
				yEdges.insert(p->imp->box.B());
			}
		}

		{
			e::Set<float>::iterator it = xEdges.begin();
			if(it != xEdges.end())
			{
				float prev = *it;
				++it;
				for(; it != xEdges.end(); ++it)
				{
					SEC sec = {prev, *it};
					prev = *it;
					xAxis.push_back(sec);
				}
			}
		}

		{
			e::Set<float>::iterator it = yEdges.begin();
			if(it != yEdges.end())
			{
				float prev = *it;
				++it;
				for(; it != yEdges.end(); ++it)
				{
					SEC sec = {prev, *it};
					prev = *it;
					yAxis.push_back(sec);
				}
			}
		}


		Pane * pane = 0;
		size_t szx = xAxis.size();
		size_t szy = yAxis.size();
		grid.resize(szy, szx, pane);

		//size_t sz = stack.size();
		for(size_t i = 0; i < sz; i++)
		{
			Pane * p = stack[i];
			Rect & rect = p->imp->box;
			int r0, r1, c0, c1;
			FindX(c0, c1, rect.L(), rect.R());
			FindY(r0, r1, rect.T(), rect.B());

			for(int x = c0; x <= c1; x++)
			{
				for(int y = r0; y <= r1; y++)
				{
					grid(y, x) = p;
				}
			}
		}

		valid = true;
	}

	Pane * PaneStack::Find(float _x, float _y)
	{
		E_ASSERT(rootPane);

		if(!rootPane)
		{
			return 0;
		}

		if(!valid)
		{
			Build();
		}

		if(_x < last_x0 || _x > last_x1)
		{
			last_i = FindX(_x);
			if(last_i == -1)
			{
				last_x0 = 1;
				last_x1 = 0;
			}
			else
			{
				last_x0 = xAxis[last_i].a;
				last_x1 = xAxis[last_i].b;
			}
			last = 0;
		}

		if(_y < last_y0 || _y > last_y1)
		{
			last_j = FindY(_y);
			if(last_j == -1)
			{
				last_y0 = 1;
				last_y1 = 0;
			}
			else
			{
				last_y0 = yAxis[last_j].a;
				last_y1 = yAxis[last_j].b;
			}
			last = 0;
		}

		if(last_i != -1 && last_j != -1)
		{
			last = grid(last_j, last_i);
		}

		return last;
	}

}

