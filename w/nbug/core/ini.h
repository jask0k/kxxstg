
#ifndef NB_CORE_INI_FILE_H
#define NB_CORE_INI_FILE_H

#include <nbug/core/str.h>
#include <nbug/core/file.h>
#include <nbug/tl/str_array.h>

namespace e
{
	class Path;
	// read write UTF-8 *.ini file
	// keys can prefix  by section:  "[section]key", empty section equal to no section, i.e. "[]key" == "key"
	class IniFile
	{
		mutable void * o;
	public:
		IniFile();
		~IniFile();
		bool Load(const Path & _path);
		bool Save(const Path & _path) const;
		void clear();
		bool empty() const;
		StringArray GetAllKey() const;
		bool GetString(string _key, string & _toLoad) const;
		void SetText(string _key, const string & _value);
		bool Get(const string & _key, string  & _toLoad)
		{ return GetString(_key, _toLoad); }
		bool Get(string _key, bool  & _toLoad);
		bool Get(string _key, int   & _toLoad);
		bool Get(string _key, float & _toLoad);
		template <typename T> void Set(const string & _key, const T & _v)
		{ SetText(_key, string(_v)); }
	};
}

#endif
