
#pragma once

#include <nbug/core/str.h>
#include <nbug/tl/array.h>

namespace e
{
	typedef Array<string> StringArray;
	StringArray Split(const stringw & _src, const wchar_t * _chs, int _from = 0, int _n = -1, bool _skip_empty = true);
}
