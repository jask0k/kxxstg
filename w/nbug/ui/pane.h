#pragma once

#include <nbug/gl/graphics.h>
#include <nbug/tl/list.h>
#include <nbug/gl/rect.h>
#include <nbug/ui/themes.h>
namespace e
{
	class Scroll;

	enum Direction
	{
		West,
		East,
		North,
		South,
	};

	class FadeFrac
	{
		float frac;
	public:
		FadeFrac() : frac(0)
		{}
		void Step(bool _inc)
		{
			if(_inc)
			{
				if(frac < 1.0f)
				{
					frac+= 0.08f;
					if(frac > 1.0f)
					{
						frac = 1.0f;
					}
				}
			}
			else 
			{
				if(frac > 0.0f)
				{
					frac-= 0.08f;
					if(frac < 0.0f)
					{
						frac = 0.0f;
					}
				}
			}
		}
		operator float() const
		{ return frac; }
	};

	class PaneStack;
	struct PaneImp;
	class Pane : public Object
	{
		friend class PaneStack;
		friend class Win;
	private:
		bool visible : 1;
	protected:
		bool popup : 1;
		bool enabled : 1;
		bool vertical : 1;
		Rect rect;
		static Pane * rootPane;
		PaneImp * imp;
		Pane * parent;
	public:
		static Themes * themes;

		static void SetRootPane(Pane * _p);
		static Pane * GetRootPane();
		static Pane * FindPane(float _x, float _y);
		static void EmulateLeftDown(float _x, float _y);
		static void Restack(bool _lazy = true);

		float ScrollX() const
		{ return 0; }
		float ScrollY() const
		{ return 0; }

		float X() const
		{ return rect.x; }
		float Y() const
		{ return rect.y; }
		float W() const
		{ return rect.w; }
		float H() const
		{ return rect.h; }

		const Rect & GetRect() const
		{ return rect; }

		void SetRect(float _x, float _y, float _w, float _h);
		void SetRect(const Rect & _r)
		{ SetRect(_r.x, _r.y, _r.w, _r.h); }


		void WinToPane(float & _x, float & _y);
		void PaneToWin(float & _x, float & _y);

		virtual void Draw();
		virtual void Step();

		Pane();
		~Pane();
		void clear();

		virtual void OnLeftDown(float _x, float _y);
		virtual void OnLeftUp(float _x, float _y);
		virtual void OnMouseMove(float _x, float _y);
		virtual void OnMouseWheel(float _x, float _y, float _dz);

		bool IsVisible() const
		{ return visible; }

		void SetVisible(bool _visible);

		void GrabMouse();
		void ReleaseMouse();
		static Pane * GetGrabMousePane();

		void SetKeyboardFocus();
		void UnsetKeyboardFocus();
		static Pane * GetKeyboardFocusPane();
		static Pane * GetMouseHoverPane();

		bool IsPopup() const
		{ return popup; }

		void SetPopup(bool _popup);

		void Raise();

		virtual Vector2 GetRequestSize() const;

		void SetParent(Pane * _parent);

		Pane * GetParent() const
		{ return parent; }

		Array<Pane*> GetChildren();
	};

}


