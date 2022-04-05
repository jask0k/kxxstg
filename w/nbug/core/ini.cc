
//#include "private.h"
#include <nbug/tl/map.h>
#include <nbug/core/debug.h>
#include <nbug/core/ini.h>
#include <nbug/core/file.h>

namespace e
{
	typedef Map<string, string> TextTextMap;

	IniFile::IniFile()
	{
		o = enew TextTextMap();
	}
	

	IniFile::~IniFile()
	{
		delete (TextTextMap*)o;
	}

	bool IniFile::Load(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		//o->modified = false;
		//o->items.clear();
		FileRef file = FS::OpenFile(_path);
		if(!file)
		{
			message(L"[nb] (WW) Failed to load ini file: \"" + _path.GetBaseName(true) + L"\"");
			return false;
		}
		// E_SCOPE_RELEASE(file);

		Charset charset = file->ReadBom();
		if(charset != CHARSET_LOCALE && charset != CHARSET_UTF8)
		{
			return false;
		}
		string section;
		stringa line0;
		string line1;
		while(file->ReadLine(line0))
		{
			line1 = string(line0, CHARSET_UTF8);
			int n = line1.find(L'#');
			if(n >= 0)
			{
				line1 = line1.substr(0, n);
			}
			line1.trim();
			if(line1.empty())
			{
				continue;
			}
			int len = line1.length();
			if(line1[0] == '[' && line1[len-1] == ']')
			{
				// treat "[]" as ""
				section = len == 2 ? L"" : line1;
			}
			else
			{
				StringArray ret = Split(line1, L"=");
				if(ret.size() == 2)
				{
					string & key = ret[0];
					key.trim();
					string & value = ret[1];
					value.trim();
					value.replace(L"\\r", L"\r");
					value.replace(L"\\n", L"\n");
					value.replace(L"\\\\", L"\\");
					(*((TextTextMap*)o))[section + key] = value;
				}
			}
		}
		return true;
	}

	bool IniFile::Save(const Path & _path) const
	{
		// NB_PROFILE_INCLUDE;
		//E_TRACE_LINE(o->path.GetString());
		if(!FS::CreateFolder(_path.GetParentFolder()))
		{
			return false;
		}
		FileRef file = FS::OpenFile(_path, true);
		if(!file)
		{
			E_ASSERT(0);
			return false;
		}
		// E_SCOPE_RELEASE(file);
		if(!file->SetSize(0))
		{
			E_ASSERT(0);
			return false;
		}
		if(!file->WriteBom(CHARSET_UTF8))
		{
			E_ASSERT(0);
			return false;
		}
		bool firstSection = true;
		TextTextMap::iterator it = ((TextTextMap*)o)->begin();
		string prevSection, section, key, value;
		for(; it != ((TextTextMap*)o)->end(); ++it)
		{
			if(it->first[0] == '[' )
			{
				int n = it->first.find(']', 1);
				if(n == -1)
				{
					E_ASSERT(0);
					continue;
				}
				section = it->first.substr(0, n+1);
				key = it->first.substr(n+1, -1);
			}
			else
			{
				section = L"";
				key = it->first;
			}

			if(section != prevSection)
			{
				if(!firstSection)
				{
					file->WriteLine(L"");
				}

				if(section.empty())
				{
					// empty section which appear after other sections must add "[]"
					if(!file->WriteLine(L"[]"))
					{
						E_ASSERT(0);
						return false;
					}
				}
				else
				{
					if(!file->WriteLine(section))
					{
						E_ASSERT(0);
						return false;
					}
				}
				firstSection = false;
				prevSection = section;
			}

			value = it->second;
			value.replace(L"\\", L"\\\\");
			value.replace(L"\r", L"\\r");
			value.replace(L"\n", L"\\n");
			value.trim();
			value = key + L" = " + value;
			if(!file->WriteLine(stringa(value,CHARSET_UTF8)))
			{
				E_ASSERT(0);
				return false;
			}
		}
		//o->modified = false;
		return true;
	}

	//bool IniFile::Save()
	//{
	//	return Save(o->path);
	//}

	void IniFile::clear()
	{
		// NB_PROFILE_INCLUDE;
		((TextTextMap*)o)->clear();
	}

	StringArray IniFile::GetAllKey() const
	{
		// NB_PROFILE_INCLUDE;
		StringArray ret;
		for(TextTextMap::iterator it = ((TextTextMap*)o)->begin(); it != ((TextTextMap*)o)->end() ; ++it)
		{
			ret.push_back(it->first);
		}
		return ret;
	}

	bool IniFile::GetString(string  _key, string & _value) const
	{
		// NB_PROFILE_INCLUDE;
		if(_key.length() >= 2 && _key[0] == '[' && _key[1] == ']')
		{
			_key = _key.substr(2, -1);
		}
		TextTextMap::iterator it = ((TextTextMap*)o)->find(_key);
		if(it != ((TextTextMap*)o)->end())
		{
			_value = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}

	void IniFile::SetText(string _key, const string & _value)
	{
		// NB_PROFILE_INCLUDE;
		if(_key.length() >= 2 && _key[0] == '[' && _key[1] == ']')
		{
			_key = _key.substr(2, -1);
		}
		TextTextMap::iterator it = ((TextTextMap*)o)->find(_key);
		if(it != ((TextTextMap*)o)->end())
		{
			if(it->second != _value)
			{
				it->second = _value;
				//o->modified = true;
			}
		}
		else
		{
			(*((TextTextMap*)o))[_key] = _value;
			//o->modified = true;
		}
	}

	bool IniFile::empty() const
	{
		return ((TextTextMap*)o)->empty();
	}

	bool IniFile::Get(string _key, bool & _toLoad)
	{
		// NB_PROFILE_INCLUDE;
		if(_key.length() >= 2 && _key[0] == '[' && _key[1] == ']')
		{
			_key = _key.substr(2, -1);
		}
		string t;
		if(GetString(_key,t))
		{
			_toLoad = t.to_bool();
			return true;
		}
		return false;
	}

	bool IniFile::Get(string _key, int & _toLoad)
	{
		// NB_PROFILE_INCLUDE;
		if(_key.length() >= 2 && _key[0] == '[' && _key[1] == ']')
		{
			_key = _key.substr(2, -1);
		}
		string t;
		if(GetString(_key,t))
		{
			_toLoad = t.to_int();
			return true;
		}
		return false;
	}

	bool IniFile::Get(string _key, float & _toLoad)
	{
		// NB_PROFILE_INCLUDE;
		if(_key.length() >= 2 && _key[0] == '[' && _key[1] == ']')
		{
			_key = _key.substr(2, -1);
		}
		string t;
		if(GetString(_key,t))
		{
			_toLoad = t.to_float();
			return true;
		}
		return false;
	}
}
