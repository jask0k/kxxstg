
//#include "private.h"
//#include <algorithm>  // sort
//#include <vector>
#include <nbug/tl/map.h>
#include <nbug/core/file.h>
#include <nbug/core/debug.h>
#include <nbug/core/ini.h>
#include <nbug/core/env.h>

namespace e
{

	typedef e::Map<string, string> DictMap;
	DictMap g_dictionary;

	bool LoadTranslation(const Path & _path, bool _clear_previous)
	{
		message(L"[nb] Load dictionary: \"" +  _path.GetBaseName(true) + L"\"");

		if(_clear_previous)
		{
			g_dictionary.clear();
		}

		IniFile ini;
		if(ini.Load(_path))
		{
			StringArray keys = ini.GetAllKey();
			for(uint i=0; i<keys.size(); i++)
			{
				const string & k = keys[i];
				string v;
				ini.Get(k, v);
				g_dictionary[k] = v;
			}
			return true;
		}
		else
		{
			return false;
		}
	}
/*
	static string ConstructItem(const string & _text)
	{
		// find first &, but not &&
		string text(_text);
		Char underLineChar = 0;
		int i = 0;
		int nPos;
		int len = text.length();
		while(true)
		{
			nPos = text.find(L'&', i);
			if(nPos == -1 ||  nPos + 1 >= len)
			{
				break;
			}

			Char ch1 =  text[nPos + 1];
			if(ch1 == L'&')
			{
				i = nPos + 2;
			}
			else
			{
				underLineChar = ch1;
				break;
			}

		}

		if(underLineChar)
		{
			//text.Delete(nPos, 1); // remove the '&'
			text = text.substr(0, nPos) + text.substr(nPos + 1, -1);
		}

		e::Map<string, string>::iterator it = g_dictionary2.find(text);
		if(it == g_dictionary2.end())
		{
#ifdef NB_DEBUG
			E_TRACE_LINE("[nb] translation not found:\t" + text );
#endif
			return _text;
		}
		else if(underLineChar)
		{
			string ret(it->second);
			int nPos = ret.find(underLineChar);
			if(nPos >= 0)
			{
				ret.insert(nPos, L'&');
			}
			else
			{
				ret.append(L"(&");
				if(underLineChar>='a' && underLineChar<='z')
				{
					underLineChar-= 'a' - 'A';
				}
				ret.append(underLineChar);
				ret.append(L')');
			}
			return ret;
		}
		else
		{
			return it->second;
		}
	}
*/
	string Translate(const string & _text)
	{
		DictMap::iterator it = g_dictionary.find(_text);
		if(it != g_dictionary.end())
		{
			return it->second;
		}
		else
		{
			// TODO: try more complex translate
			g_dictionary[_text] = _text;
			return _text;
		}
	}
}
