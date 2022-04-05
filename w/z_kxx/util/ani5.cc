
// #include "../config.h"
#include <nbug/core/debug.h>
#include <nbug/core/obj.h>
#include <z_kxx/util/ani5.h>
#include <nbug/gl/graphics.h>
#include <z_kxx/main/kxxwin.h>

namespace e
{
	class Ani5Imp : public RefObject
	{
	public:
		static const int MAX_FRAME = 8;
		TexRef tex[5][MAX_FRAME];
		//bool flip_x[5][MAX_FRAME];
		int size[5];
		int span;
		void StepCenter(Ani5 * _s);
		void StepLeft(Ani5 * _s);
		void StepRight(Ani5 * _s);
	};

	inline void Ani5Imp::StepCenter(Ani5 * _s)
	{
		int & pos0  = _s->pos0;
		int & pos1  = _s->pos1;
		switch(pos0)
		{
		case 0:
			pos0 = 1;
			pos1 = size[1] - 1;
			break;
		case 1:
			pos1--;
			if(pos1 < 0)
			{
				pos0 = 2;
				pos1 = 0;
			}
			break;
		case 2:
			pos1++;
			if(pos1 >= size[2])
			{
				pos1 = 0;
			}
			break;
		case 3:
			pos1--;
			if(pos1 < 0)
			{
				pos0 = 2;
				pos1 = 0;
			}
			break;
		case 4:
			pos0 = 3;
			pos1 = size[1] - 1;
			break;
		}	
	}

	inline void Ani5Imp::StepLeft(Ani5 * _s)
	{
		int & pos0  = _s->pos0;
		int & pos1  = _s->pos1;
		switch(pos0)
		{
		case 0:
			pos1++;
			if(pos1 >= size[0])
			{
				pos1 = 0;
			}
			break;
		case 1:
			pos1++;
			if(pos1 >= size[1])
			{
				pos0 = 0;
				pos1 = 0;
			}
			break;
		case 2:
			pos0 = 1;
			pos1 = 0;
			break;
		case 3:
			pos1--;
			if(pos1 < 0)
			{
				pos0 = 1;
				pos1 = 0;
			}
			break;
		case 4:
			pos0 = 3;
			pos1 = size[1] - 1;
			break;
		}	
	}

	inline void Ani5Imp::StepRight(Ani5 * _s)
	{
		int & pos0  = _s->pos0;
		int & pos1  = _s->pos1;
		switch(pos0)
		{
		case 0:
			pos0 = 1;
			pos1 = size[1] - 1;
			break;
		case 1:
			pos1--;
			if(pos1 < 0)
			{
				pos0 = 3;
				pos1 = 0;
			}
			break;
		case 2:
			pos0 = 3;
			pos1 = 0;
			break;
		case 3:
			pos1++;
			if(pos1 >= size[3])
			{
				pos0 = 4;
				pos1 = 0;
			}
			break;
		case 4:
			pos1++;
			if(pos1 >= size[4])
			{
				pos1 = 0;
			}
			break;
		}
	}

	TexRef Ani5::GetTex()
	{ 
		return imp ? imp->tex[pos0][pos1] : kxxwin->badTex;
	}

	void Ani5::Step(int _dir)
	{ 
		E_ASSERT(imp);
		E_ASSERT(timer);
		timer--;
		if(timer > 0)
		{
			return;
		}

		timer = imp->span;

		// frame switch
		if(_dir == 0)
		{
			imp->StepCenter(this);
		}
		else if(_dir < 0)
		{
			imp->StepLeft(this);
		}
		else 
		{
			imp->StepRight(this);
		}	
	}

	Ani5::Ani5(const Ani5 & _r)
	{
		if(_r.imp)
		{
			imp = _r.imp;
			imp->AddRef();
			pos0 = _r.pos0;
			pos1 = _r.pos1;
			timer = _r.timer;
		}
		else
		{
			imp = 0;
		}

	}

	const Ani5 & Ani5::operator=(const Ani5 & _r)
	{
		if(this != &_r)
		{
			Detach();
			if(_r.imp)
			{
				imp = _r.imp;
				imp->AddRef();
				pos0 = _r.pos0;
				pos1 = _r.pos1;
				timer = _r.timer;
			}
			else
			{
				imp = 0;
			}
		}
		return *this;
	}

	Ani5::~Ani5()
	{
		Detach();
	}


	void Ani5::Detach()
	{
		if(imp)
		{
			imp->Release();
			imp = 0;
		}
	}

	bool Ani5::Load(Graphics * _g, uint _fps, const Path & _path)
	{
		Detach();

		E_ASSERT(_g);
		E_ASSERT(_fps > 0);

		FileRef file = FS::OpenFile(_path);
		if(!file)
		{
			message(L"[kx] (WW) Ani5::Load(): Failed to open file: " + _path.GetBaseName());
			return false;
		}

		Ani5Imp * p = enew Ani5Imp;
		p->size[0] = 1;
		p->size[1] = 1;
		p->size[2] = 1;
		p->size[3] = 1;
		p->size[4] = 1;
		p->span = 1;


		stringa lineA;
		string line;
		StringArray words;
			
		int readPos0 = -1;
		int readPos1 = 0;
		bool loadNextPos0 = true;
		bool firstLine = true;
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
			E_ASSERT(wordCount >= 1);

			if(firstLine)
			{
				firstLine = false;
				p->span = lineA.to_int(); // ms
				p->span = int(float(p->span) * float(_fps) / 1000 + 0.5f);
				if(p->span < 1)
				{
					p->span = 1;
				}
				//timer = span;
			}
			else if(loadNextPos0) 
			{
				loadNextPos0 = false;
				readPos0++;
				readPos1 = 0;
				if(readPos0 >= 0 && readPos0 < 5)
				{
					p->size[readPos0] = words[0].to_int();
					E_ASSERT(p->size[readPos0] > 0);
					if(p->size[readPos0] < 1)
					{
						loadNextPos0 = true;
					}
				}
			}
			else if(readPos0 >= 0 && readPos0 < 5)
			{
				bool flipx = false;
				bool flipy = false;
				if(wordCount > 1)
				{
					if(words[1].icompare(L"fx") == 0)
					{
						flipx = true;
					}
					else if(words[1].icompare(L"fy") == 0)
					{
						flipy = true;
					}
					else if(words[1].icompare(L"fxy") == 0)
					{
						flipx = true;
						flipy = true;
					}
					else
					{
						message(L"[kx] (WW) Invalid flip option: " + _path.GetBaseName() + L"(" + string(line_num) + L"): \"" + line + L"\"");
					}
				}

				TexRef & tex = p->tex[readPos0][readPos1];
				tex = kxxwin->LoadTex(words[0], true);
				if(tex)
				{
					tex = tex->Clone();
					tex->flip_x = flipx;
					tex->flip_y = flipy;
				}
				readPos1++;
				if(readPos1 >= p->size[readPos0])
				{
					loadNextPos0 = true;
					readPos1 = 0;
				}
			}
			else
			{
				message(L"[kx] (WW) Sub ani count > 5: " +_path.GetBaseName() + L"(" + string(line_num) + L"): \"" + line + L"\"");
				break;
			}
		}
		imp = p;
		imp->AddRef();
		pos0 = 2;
		pos1 = 0;
		timer = imp->span;
		return true;
	}
}
