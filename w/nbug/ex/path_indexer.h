
#pragma once
#include <nbug/core/str.h>
#include <nbug/core/file.h>

namespace e
{
	class PathIndexerImp;
	class PathIndexer
	{
		PathIndexerImp * imp;
	public:
		PathIndexer(const Path & _folder, const string & _patterns) throw();
		~PathIndexer();
		Path GetPath(const string & _baseName) const;
		bool IsValid() const
		{ return imp != 0; }
	};
}

