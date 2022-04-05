#include <nbug/gl/private.h>
#include <nbug/ui/pane.h>
#include <nbug/ui/win.h>
#include <nbug/ui/win_.h>
#include <nbug/gl/rect.h>
#include <nbug/ui/pane_stack.h>
#include <nbug/ui/pane_.h>
//#include "Scroll.h"

namespace e
{
	Themes * Pane::themes = 0;

	Pane * Pane::rootPane = 0;
	PaneStack * g_pane_stack = 0;
	Pane * g_keyboard_focus_pane = 0;
	Pane * g_grab_mouse_pane = 0;
	Array<Pane *>  * g_keyboard_focus_pane_stack = 0;
	Array<Pane *>  * g_grab_mouse_pane_stack = 0;
	Pane * g_mouse_hover_pane = 0;
	List<Pane*> * g_noparent_pane_list = 0;


	static inline  Pane * _PopWedgetStack(Array<Pane*> * _stack, Pane * _w, bool _topOnly)
	{
		int last;
_AGAIN:
		E_ASSERT(_w);
		last = -1;
		bool removed = false;
		if(!_topOnly)
		{
			for(size_t i = 0; i < _stack->size(); i++)
			{
				Pane * & p = (*_stack)[i];
				if(p == _w)
				{
					removed = true;
					p = 0;
				}
				else if(p != 0)
				{
					last = i;
				}
			}
		}
		else
		{
			for(int i = _stack->size() - 1; i >= 0; i--)
			{
				Pane * & p = (*_stack)[i];
				if(p == _w)
				{
					removed = true;
					p = 0;
				}
				else if(p != 0)
				{
					break;
					last = i;
				}
			}

		}
		if(removed)
		{
			//E_TRACE_LINE("[e_ui] Win_o::PopGrabMousePane(): removed");
			_stack->resize(last+1);
			if(last >= 0)
			{
				_w = (*_stack)[last];
				if(_w->IsVisible())
				{
					return _w;
				}
				else if(!_topOnly)
				{
					goto _AGAIN;
				}

			}
		}
		return 0;
	}

	static void CleanUp()
	{
		for(List<Pane*>::iterator it = g_noparent_pane_list->begin(); it != g_noparent_pane_list->end(); )
		{
			Pane * p = *it;
			if(p != Pane::GetRootPane())
			{
				it = g_noparent_pane_list->erase(it);
				delete p;
			}
			else
			{
				++it;
			}
		}
		delete g_keyboard_focus_pane_stack;
		delete g_grab_mouse_pane_stack;
		delete g_noparent_pane_list;
		delete g_pane_stack;
		g_noparent_pane_list = 0;
	}

	Pane::Pane()
	{
		imp = enew PaneImp;
		parent = 0;
		popup = false;
		visible = true;
		enabled = true;
		vertical = false;
		if(g_noparent_pane_list == 0)
		{
			g_pane_stack = new PaneStack();
			g_keyboard_focus_pane_stack = new Array<Pane*>();
			g_grab_mouse_pane_stack = new Array<Pane*>();
			g_noparent_pane_list = enew List<Pane*>();
			atexit(&CleanUp);
		}
		g_noparent_pane_list->push_back(this);
	}

	Pane::~Pane()
	{
		if(parent == 0)
		{
			g_noparent_pane_list->erase(this);
		}
		clear();
		delete imp;
	}

	void Pane::clear()
	{
		for(List<Pane*>::iterator it = imp->children.begin(); it != imp->children.end(); ++it)
		{
			delete *it;
		}
		imp->children.clear();
	}

	void Pane::Draw()
	{
	}

	void Pane::Step()
	{}



	void Pane::OnLeftDown(float _x, float _y)
	{
		E_TRACE_LINE(L"[nb] Pane::OnLeftDown(" + string(_x) + L", " + string(_y) + L")");
	}

	void Pane::OnLeftUp(float _x, float _y)
	{
		E_TRACE_LINE(L"[nb] Pane::OnLeftUp(" + string(_x) + L", " + string(_y) + L")");
	}

	void Pane::OnMouseMove(float _x, float _y)
	{
		//E_TRACE_LINE(L"[nb] Pane::OnMouseMove(" + string(_x) + L", " + string(_y) + L")");
	}

	void Pane::OnMouseWheel(float _x, float _y, float _dz)
	{
		E_TRACE_LINE(L"[nb] Pane::OnMouseWheel(" + string(_x) + L", " + string(_y) + L", " + string(_dz) + L")");
	}

	void Pane::GrabMouse()
	{
		if(visible && g_grab_mouse_pane != this)
		{
			g_grab_mouse_pane = this;
			g_grab_mouse_pane_stack->push_back(this);
		}
	}

	void Pane::ReleaseMouse()
	{
		g_grab_mouse_pane = _PopWedgetStack(g_grab_mouse_pane_stack, this, false);
	}

	Pane * Pane::GetGrabMousePane()
	{ return g_grab_mouse_pane; }

	void Pane::SetKeyboardFocus()
	{
		if(visible && g_keyboard_focus_pane != this)
		{
			g_keyboard_focus_pane = this;
			g_keyboard_focus_pane_stack->push_back(this);
		}
	}

	void Pane::UnsetKeyboardFocus()
	{
		g_keyboard_focus_pane = _PopWedgetStack(g_keyboard_focus_pane_stack, this, false);
	}

	Pane * Pane::GetKeyboardFocusPane()
	{ return g_keyboard_focus_pane; }

	//bool Pane::HasKeyboardFocus() const
	//{
	//	Pane * w = g_keyboard_focus_pane;
	//	return w == this;
	//}

	Pane * Pane::GetMouseHoverPane()
	{ return g_mouse_hover_pane; }

	void Pane::SetPopup(bool _popup)
	{
		if(popup != _popup)
		{
			popup = _popup;
			Restack();
		}
	}

	void Pane::WinToPane(float & _x, float & _y)
	{
		g_pane_stack->Validate();
		_x-= imp->offset.x;
		_y-= imp->offset.y;
	}

	void Pane::PaneToWin(float & _x, float & _y)
	{
		g_pane_stack->Validate();
		_x+= imp->offset.x;
		_y+= imp->offset.y;
	}

	void Pane::Raise()
	{
		Pane * p = this;
		while(p->parent)
		{
			//p->parent->RaiseChild(i);
			p->parent->imp->children.erase(p);
			p->parent->imp->children.push_back(p);
			p = p->parent;
		}
		if(parent)
		{
			Restack();
		}
	}

	Vector2 Pane::GetRequestSize() const
	{
		Vector2 ret = {4, 4};
		return ret;
	}

	void Pane::SetRect(float _x, float _y, float _w, float _h)
	{
		rect.x = _x;
		rect.y = _y;
		rect.w = _w;
		rect.h = _h;
		Restack();
	}

	void Pane::SetVisible(bool _visible)
	{
		if(_visible != visible)
		{
			visible = _visible;
			if(!_visible)
			{
				if(g_keyboard_focus_pane == this)
				{
					UnsetKeyboardFocus();
				}
				if(g_grab_mouse_pane == this)
				{
					ReleaseMouse();
				}
			}
			Restack();
		}
	}

	void Pane::SetRootPane(Pane * _p)
	{
		if(Pane::rootPane != _p)
		{
			delete Pane::rootPane;
			Pane::rootPane = _p;
			if(_p)
			{
				_p->parent = 0;
				g_keyboard_focus_pane = 0;
				g_grab_mouse_pane = 0;
				g_mouse_hover_pane = 0;
				if(Pane::themes == 0)
				{
					Pane::themes = enew Themes(Graphics::Singleton());
				}

				g_pane_stack->SetRoot(_p);
			}
			else
			{
				if(Pane::themes)
				{
					g_pane_stack->Invalidate();
					g_keyboard_focus_pane = 0;
					g_grab_mouse_pane = 0;
					g_keyboard_focus_pane_stack->clear();
					delete Pane::themes;
					Pane::themes = 0;
				}
			}
		}
	}

	Pane * Pane::GetRootPane()
	{
		return Pane::rootPane;
	}

	Pane * Pane::FindPane(float _x, float _y)
	{
		return g_pane_stack->Find(_x, _y);
	}

	void Pane::Restack(bool _lazy)
	{
		g_pane_stack->Invalidate();
		if(!_lazy)
		{
			g_pane_stack->Validate();
		}
	}

	void Pane::SetParent(Pane * _parent)
	{
		if(_parent != parent)
		{
			if(parent)
			{
				parent->imp->children.erase(this);
				if(_parent == 0)
				{
					g_noparent_pane_list->push_back(this);
				}
			}

			if(_parent)
			{
				_parent->imp->children.push_back(this);
				if(parent == 0)
				{
					g_noparent_pane_list->erase(this);
				}
			}

			parent = _parent;
			Restack();
		}
	}

	Array<Pane*> Pane::GetChildren()
	{
		Array<Pane*> ret;
		for(List<Pane*>::iterator it = imp->children.begin(); it != imp->children.end(); ++it)
		{
			ret.push_back(*it);
		}
		return ret;
	}

	void Pane::EmulateLeftDown(float _x, float _y)
	{
		Win::singleton->OnLeftDown(_x, _y);
	}
}

