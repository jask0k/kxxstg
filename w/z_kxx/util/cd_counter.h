
#pragma once

namespace e
{
	struct CDCounter
	{
		float m;
		float c;
		void Set(float _t)
		{ 
			m = _t;
			c = 0; 
		}

		void Reset()
		{ c = 0; }

		float Ratio() const
		{ return c / m; }

		bool Step()
		{ 
			c+= 1.0f;
			if(c > m)
			{
				c = m;
				return false;
			}
			else
			{
				return true;
			}
		}
	};
}
