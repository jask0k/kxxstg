
//#include "private.h"
#include <nbug/tl/str_array.h>

namespace e
{
	StringArray Split(const stringw & _src, const wchar_t * _chs, int _from, int _n, bool _skip_empty)
	{
		StringArray ret;
		int len = _src.length();
		if(_n!= -1 && _from + _n < len)
		{
			len = _from + _n;
		}
		int pos = _from;
		while(pos < len)
		{
			int n = _src.find_any(_chs, pos);
			if(n == -1)
			{
				stringw t = _src.substr(pos, -1);
				if(!_skip_empty || !t.empty())
				{
					ret.push_back(t);
					break;
				}
			}
			else
			{
				stringw t = _src.substr(pos, n - pos);
				if(!_skip_empty || !t.empty())
				{
					ret.push_back(t);
				}
				pos = n + 1;
			}
		}
		return ret;
	}

}
