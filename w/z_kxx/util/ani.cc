
// #include "../config.h"
#include <string.h>
#include <z_kxx/util/ani.h>
#include <nbug/gl/graphics.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	class Tex;
	class AniImp : public RefObject
	{
	public:
		struct AniFrame
		{
			//E_USE_MPOOL(AniFrame);
			TexRef tex;
			uint span;
		};
		void _Init(AniFrame _frames[], size_t _size) ;
		AniImp();
		~AniImp();
		AniFrame * frames;
		size_t size;
		uint totalSpan;
		int init_loop;
	};

	AniImp::AniImp()
	{
		frames = 0;
		size = 0;
	}

	AniImp::~AniImp()
	{
		delete[] frames;
	}

	void AniImp::_Init(AniFrame _frames[], size_t _size)
	{
		E_ASSERT(_size > 0);
		init_loop = 0;
		totalSpan = 0;
		size = _size;
		frames = enew AniFrame[_size];
		for(size_t i=0; i<_size; i++)
		{
			frames[i] = _frames[i];
			totalSpan+= frames[i].span;
		}
	}

	struct LOAD_ANI_ITEM
	{
		int ms;
		char name[64];
		bool flipx;
		bool flipy;
	};


	TexRef Ani::GetTex()
	{
		return imp ? imp->frames[index].tex : kxxwin->badTex;
	}

	void Ani::Reset()
	{
		loop = imp->init_loop;
		index = 0;
		timer = imp->frames[index].span;
	}

	bool Ani::Step()
	{
		E_ASSERT(imp);

		if(timer == 0)
		{
			return false;
		}

		E_ASSERT(index >= 0 && index < (int)imp->size);

		if(--timer)
		{
			return true;
		}

		if(index < (int)imp->size - 1)
		{
			index++;
			timer = imp->frames[index].span;
			return true;
		}

		if(loop)
		{
			if(loop>0)
			{
				loop--;
			}
			index = 0;
			timer = imp->frames[index].span;
			return true;
		}
		
		return false;
	}

	/*
	bool Ani::StepBack()
	{
		E_ASSERT(timer > 0 && index >= 0 && index < (int)imp->size);
		bool ret;
		if(--timer == 0)
		{
			if(--index < 0)
			{
				index = imp->size - 1;
				ret = false;
			}
			else
			{
				ret = true;
			}
			timer = imp->frames[index].span;
		}
		else
		{
			ret = true;
		}
		return ret;
	}
	*/


	Ani::Ani(const Ani & _r)
	{
		if(_r.imp)
		{
			imp = _r.imp;
			imp->AddRef();
			index = _r.index;
			timer = _r.timer;
		}
		else
		{
			imp = 0;
		}
	}

	const Ani & Ani::operator=(const Ani & _r)
	{
		if(this != &_r)
		{
			Detach();
			if(_r.imp)
			{
				imp = _r.imp;
				imp->AddRef();
				index = _r.index;
				timer = _r.timer;
			}
			else
			{
				imp = 0;
			}
		}
		return *this;
	}

	void Ani::Detach()
	{
		if(imp)
		{
			imp->Release();
			imp = 0;
		}
	}

	uint Ani::GetTotalSpan() const
	{
		return imp ? imp->totalSpan : 0;
	}

	bool Ani::Load(Graphics * _g, uint _fps, const Path & _path)
	{
		Detach();

		if(_g == 0)
		{
			E_ASSERT(0);
			return 0;
		}

		if(_fps == 0)
		{
			E_ASSERT(0);
			_fps = 1;
		}

		int loop = 0;
		Array<LOAD_ANI_ITEM> items;

		{
			FileRef file = FS::OpenFile(_path, false);
			if(!file)
			{
				message(L"[kx] (WW) Ani::Load(): Failed to open file: " + _path.GetBaseName());
				return false;
			}

			stringa lineA;
			string line;
			StringArray words;

			int line_num = 0;
			while(file->ReadLine(lineA))
			{
				line_num++;
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

				if(wordCount < 2)
				{
					message(L"[kx] (WW) Word count < 2: " + _path.GetBaseName() + L"(" + string(line_num) + L"): \"" + line + L"\"");
					continue;
				}
				if(words[0].icompare(L"loop") == 0)
				{
					loop = words[1].to_int();
					continue;
				}
				stringa nameA = words[1];
				if(nameA.length() >= 64)
				{
					//E_TRACE_LINE(_path.GetString() + L"(" + string(line_num) + L"): (WW) Name length >= 64: \"" + line + L"\"");
					message(L"[kx] (WW) length() of name >= 64: " + _path.GetBaseName() + L"(" + string(line_num) + L"): \"" + line + L"\"");
					continue;
				}

				LOAD_ANI_ITEM item;
				item.ms = words[0].to_int();
				item.flipx = false;
				item.flipy = false;
				if(wordCount > 2)
				{
					if(words[2].icompare(L"fx") == 0)
					{
						item.flipx = true;
					}
					else if(words[2].icompare(L"fy") == 0)
					{
						item.flipy = true;
					}
					else if(words[2].icompare(L"fxy") == 0)
					{
						item.flipx = true;
						item.flipy = true;
					}
					else
					{
						message(L"[kx] (WW) Invalid flip option: " + _path.GetBaseName() + L"(" + string(line_num) + L"): \"" + line + L"\"");
					}
				}
				strcpy(item.name, nameA.c_str());
				items.push_back(item);
			}
		}

		if(items.empty())
		{
			message(L"[kx] (WW) Nothing defined in this file: " + _path.GetBaseName() + L": ");
			return false;
		}

		size_t sz = items.size();
		AniImp::AniFrame * frames = enew AniImp::AniFrame[sz];
		for(size_t i = 0; i < sz; i++)
		{
			LOAD_ANI_ITEM & item = items[i];
			TexRef & tex = frames[i].tex;
			tex = kxxwin->LoadTex(item.name);
			if(tex)
			{
				tex = tex->Clone();
				tex->flip_x = item.flipx;
				tex->flip_y = item.flipy;
			}

			frames[i].span = item.ms <= 0
				? 0x7fffffff
				: (uint)(item.ms * _fps /1000.0f  + 0.4999f);
			E_ASSERT(frames[i].span > 0);
			if(frames[i].span == 0)
			{
				frames[i].span = 1;
			}
		}

		imp = enew AniImp();
		imp->_Init(frames, sz);
		imp->init_loop = 0;
		delete[] frames;
		index = 0;
		loop = imp->init_loop;
		timer = imp->frames[0].span;
		imp->AddRef();
		return true;
	}

	//Ani AniImp::Create(AniFrame _frames[], size_t _size)
	//{
	//	if(_size < 1)
	//	{
	//		E_ASSERT(0);
	//		return 0;
	//	}

	//	AniImp * p = enew AniImp();
	//	p->_Init(_frames, _size);
	//	return p;
	//}

}
