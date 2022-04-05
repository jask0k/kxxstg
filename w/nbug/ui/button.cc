#include <nbug/ui/button.h>

namespace e
{
	Button::Button()
	{
		leftDown = false;
	}

	void Button::Draw()
	{
		themes->DrawButton(rect.w, rect.h, pushFrac, hoverFrac);
		themes->DrawLabel(rect.w, rect.y, text);
	}

	void Button::Step()
	{
		bool hover = GetMouseHoverPane() == this;
		hoverFrac.Step(hover);
		pushFrac.Step(leftDown && hover);
	}
	
	void Button::OnLeftDown(float _x, float _y)
	{
		GrabMouse();
		leftDown = true;
	}

	void Button::OnLeftUp(float _x, float _y)
	{
		bool hover = GetMouseHoverPane() == this;
		bool click = leftDown && hover;
		leftDown = false;
		ReleaseMouse();
		if(click)
		{
			onClick(this);
		}
	}
}
