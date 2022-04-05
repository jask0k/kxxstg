#pragma once

#include <nbug/tl/array.h>
#include <nbug/tl/array2.h>
#include <nbug/gl/rect.h>
#include <nbug/math/math.h>

namespace e
{
	class Graphics;
	class Pane;
	struct Rect;
	class PaneStack
	{
		struct SEC
		{
			float a;
			float b;
		};

		Array<SEC> xAxis;
		Array<SEC> yAxis;

		Array2<Pane *> grid;
		Pane * last;
		int last_i;
		float last_x0;
		float last_x1;
		int last_j;
		float last_y0;
		float last_y1;

		Pane * rootPane;
		bool valid;

		void FindX(int & _0, int & _1, float _x0, float _x1);
		void FindY(int & _0, int & _1, float _y0, float _y1);
		int FindX(float _x);
		int FindY(float _y);
		void Build();
		//void RecursiveMarkCell(Pane * _pane, const Rect * _clip);
		void RecursiveAddPane(Pane * _p, int _pass, float offx, float offy, Rect * _clip);
		static void RecursiveMarkToAdd(Pane * _p);
	public:
		//struct F
		//{
		//	Pane * pane;
		//	//float offset_x;
		//	//float offset_y;
		//	//Rect box;
		//};
		void SetRoot(Pane * _root);
		void Invalidate();
		void Validate()
		{ if(!valid) Build(); }
		bool IsValid() const
		{ return valid; }
		Pane * Find(float _x, float _y);
		PaneStack();
		~PaneStack();
		Array<Pane *> stack;
	};

}
