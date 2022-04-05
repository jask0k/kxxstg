#pragma once

#include <nbug/ui/pane.h>
#include <nbug/core/callback.h>

namespace e
{
	class Button : public Pane
	{
		FadeFrac pushFrac, hoverFrac;
		bool leftDown;
	public:
		string text;
		Callback onClick;
		Button();
		void Draw() override;
		void Step() override;
		void OnLeftDown(float _x, float _y) override;
		void OnLeftUp(float _x, float _y) override;
	};
}
