#pragma once

namespace e
{
	struct PaneImp
	{
		bool inStack : 1;
		Vector2 offset;
		Rect box;
		List<Pane*> children;
	};
}
