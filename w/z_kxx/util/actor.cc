
// #include "../config.h"
#include <z_kxx/util/actor.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	Actor::Actor()
	{
		for(int i=0; i<8; i++)
		{
			emotionTex[i] = 0;
			emotionTexLoadFailed[i] = false;
		}
		nameTex = 0;
		nameTexLoadFailed = false;
	}

	Actor::~Actor()
	{
		//for(int i=0; i<8; i++)
		//{
		//	if(emotionTex[i] != 0 )
		//	{
		//		emotionTex[i]->Release();
		//	}
		//}
		//if(nameTex)
		//{
		//	nameTex->Release();
		//}
	}

	Actor * Actor::Create(const string & _short_name)
	{

		FileRef file = kxxwin->OpenResFile( Path(L"./") | L"scenario/actors.txt");
		if(!file)
		{
			error(L"[kx] Failed to load file: actors.txt");
			return 0;
		}
		// E_SCOPE_RELEASE(file);

		stringa lineA;
		string line;
		StringArray words;
		while(file->ReadLine(lineA))
		{
			line = string(lineA, CHARSET_UTF8);
			int n = line.find(L'#');
			if(n != -1)
			{
				line = line.substr(0, n);
			}
			line.trim();
			if(line.empty())
			{
				continue;
			}
			words = Split(line, L" \t");
			int wordCount = words.size();

			if(words[1].icompare(_short_name) ==0)
			{
				Actor * ret = enew Actor();
				words[0].trim();
				words[1].trim();
				ret->isEnemy = words[0].to_bool();
				ret->short_name = words[1];
				ret->name = Translate(words[2]);
				return ret;
			}
		}
	
		E_ASSERT(0);
		return 0;
	}

	TexRef Actor::LoadEmotionTex(int _index)
	{
		//E_TRACE_LINE(string(_index));
		E_ASSERT(_index >= 0 && _index < 8);
		if(!emotionTex[_index] && !emotionTexLoadFailed[_index])
		{
			//stringa sindex = stringa::format("%d", _index);
			stringa texName(stringa(short_name) + "-" + stringa(_index));
			emotionTex[_index] = kxxwin->LoadTex(texName.c_str());
			if(!emotionTex[_index])
			{
				emotionTexLoadFailed[_index] = true;
			}
		}
		return emotionTex[_index];
	}

	TexRef Actor::LoadNameTex()
	{
		if(!nameTex && !nameTexLoadFailed)
		{
			stringa texName(short_name + string("-name"));
			nameTex = kxxwin->LoadTex(texName.c_str());
			if(!nameTex)
			{
				nameTexLoadFailed = true;
			}
		}
		return nameTex;
	}
}
