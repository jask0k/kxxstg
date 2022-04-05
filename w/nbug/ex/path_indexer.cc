
//#include "private.h"
#include <nbug/tl/map.h>
#include <nbug/ex/path_indexer.h>
#include <nbug/core/debug.h>
#include <nbug/gl/image.h>

namespace e
{	
	typedef e::Map<stringa, Path> StringPathMap;
#ifdef NB_DEBUG
	typedef e::Map<stringa, bool> StringBoolMap;
#endif
	class PathIndexerImp
	{
	public:
		// FSRef fs;
		Path rootFolder;
		StringPathMap pathMap;
#ifdef NB_DEBUG
		StringBoolMap inPackMap;
#endif
	};

	PathIndexer::PathIndexer(const Path & _folder, const string & _patterns) throw()
	{
		imp = 0;
		StringArray pattern_array = Split(_patterns, L":");
		DirectoryRef dir = FS::OpenDir(_folder);
		if(!dir)
		{
			message(L"[nb] (WW) PathIndexer: Failed to open dir: \"" + _folder.GetBaseName() + L"\"");
			return;
		}
		imp = enew PathIndexerImp;
		imp->rootFolder = _folder;
		do
		{
			if(dir->IsDir())
			{
				continue;
			}
			for(size_t i=0; i<pattern_array.size(); i++)
			{
				if(MatchFileName(dir->GetName(), pattern_array[i]))
				{
					string name = Path(dir->GetName()).GetBaseName(false);
					if(name[0] == L'!')
					{
						name = name.substr(1, -1);
					}
#ifdef NB_DEBUG
					bool inpack = dir->IsInFilePack();
					StringPathMap::iterator it = imp->pathMap.find(name);
					if(it != imp->pathMap.end())
					{
						bool inpack2 = imp->inPackMap[name];
						message(L"[nb] (WW) PathIndexer: ambiguous file: " + QuoteString(name));
						message(L"    => " + it->second.GetString() + (inpack2? L" (PACK)" : L" (FS)"));
						message(L"    => " + dir->GetRelativePath().GetString() + (inpack? L" (PACK)" : L" (FS)"));
					}
					else
					{
						imp->inPackMap[name] = inpack;
						imp->pathMap[name] = dir->GetRelativePath();
					}
#else
					imp->pathMap[name] = dir->GetRelativePath();
#endif
					break;
				}
			}
		}while(dir->MoveNext(true));
	}


	PathIndexer::~PathIndexer()
	{
		delete imp;
	}

	Path PathIndexer::GetPath(const string & _baseName) const
	{
		if(imp)
		{
			StringPathMap::iterator it = imp->pathMap.find(_baseName);
			if(it == imp->pathMap.end())
			{
				return Path();
			}
			return imp->rootFolder | it->second;
		}
		else
		{
			return Path();
		}
	}

}
