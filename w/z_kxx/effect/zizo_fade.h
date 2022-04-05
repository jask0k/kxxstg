
#pragma once

#include "fade.h"

namespace e
{
	// zoom in zoom out
	class ZIZOFade : public Fade
	{
		bool zoomX;
		bool zoomY;
	public:
		ZIZOFade(bool _x = true, bool _y = true);
		~ZIZOFade();
		void Render() override;
	};
}
