#include <stdio.h>
#ifdef NB_WINDOWS
#	include <windows.h>
#endif
#include <nbug/core/str.h>
#include <nbug/core/ini.h>
#include <nbug/core/file.h>
#include <nbug/core/env.h>
#include <nbug/core/zip.h>
//#include <nbug/gl/image.h>

using namespace e;

#pragma pack(push, 1)

inline static bool nb_png_read_uint16(FileRef & file, uint16 & _n)
{
	uint8 buf[2];
	if(file->Read(buf, 2))
	{
		_n = uint32(buf[0]) << 8 | uint32(buf[1]);
		return true;
	}
	else
	{
		E_ASSERT(0); return false;
	}
}

inline static bool nb_png_read_uint32(FileRef & file, uint32 &_n)
{
	uint8 buf[4];
	if(file->Read(buf, 4))
	{
		_n = uint32(buf[0]) << 24 | uint32(buf[1]) << 16 | uint32(buf[2]) << 8 | uint32(buf[3]);
		return true;
	}
	else
	{
		E_ASSERT(0); return false;
	}
}

inline static bool nb_png_write_uint16(FileRef & file, uint16 _n)
{
	uint8 * buf = (uint8*) &_n;
	return file->Write(buf+1, 1) && file->Write(buf+0, 1);
}

inline static bool nb_png_write_uint32(FileRef & file, uint32 _n)
{
	uint8 * buf = (uint8*) &_n;
	return file->Write(buf+3, 1) && file->Write(buf+2, 1) && file->Write(buf+1, 1) && file->Write(buf+0, 1);
}

static uint8 nb_png_sig[] = {137, 80, 78, 71, 13, 10, 26, 10};

static bool check_signature(FileRef & file)
{
	static uint8 buf[8];

	if(!file->Read(buf, 8) || memcmp(buf, nb_png_sig, 8) != 0)
	{
		E_ASSERT(0); return false;
	}
	return true;
}

static bool write_signature(FileRef & file)
{
	return file->Write(nb_png_sig, 8);
}

static const int APNG_DISPOSE_OP_NONE       = 0;
static const int APNG_DISPOSE_OP_BACKGROUND = 1;
static const int APNG_DISPOSE_OP_PREVIOUS   = 2;

static const int APNG_BLEND_OP_SOURCE       = 0;
static const int APNG_BLEND_OP_OVER         = 1;

struct IHDR
{
	uint32 length;
	uint8 name[4]; //IHDR
	uint32 width;
	uint32 height;
	uint8 bit_depth;
	uint8 color_type;
	uint8 compress;
	uint8 filter;
	uint8 interlace;
	uint8 crc[4];


	bool read(FileRef & file)
	{
		if(!nb_png_read_uint32(file, length) || length != 13)
		{
			E_ASSERT(0);
			E_ASSERT(0); return false;
		}

		// first chunk must be IHDR
		if(!file->Read(name, 4) || memcmp(name, "IHDR", 4) != 0)
		{
			E_ASSERT(0);
			E_ASSERT(0); return false;
		}

		if(	!nb_png_read_uint32(file, width)
			|| !nb_png_read_uint32(file, height)
			|| !file->Read(&bit_depth, 5 + 4))
		{
			E_ASSERT(0); return false;
		}
		// TODO: verify
		return true;
	}

	bool write(FileRef & file)
	{
		if(!nb_png_write_uint32(file, 13)
			|| !file->Write("IHDR", 4)
			|| !nb_png_write_uint32(file, width)
			|| !nb_png_write_uint32(file, height)
			|| !file->Write(&bit_depth, 5 + 4))
		{
			E_ASSERT(0); return false;
		}
		return true;
	}
};

bool check_ihdr_compatible(const IHDR & _main, const IHDR & _frame)
{
	if(_frame.width > _main.width || _frame.height > _main.height)
	{
		E_ASSERT(0); return false;
	}

	if(_frame.bit_depth != _main.bit_depth
		|| _frame.color_type != _main.color_type
		|| _frame.compress != _main.compress
		|| _frame.filter != _main.filter
		|| _frame.interlace != _main.interlace)
	{
		E_ASSERT(0); return false;
	}

	return true;
}

struct ACTL
{
	uint32 num_frames;
	uint32 num_loop;
	uint32 crc;
	ACTL()
	{
		num_frames = 0;
		num_loop = 0;
	}
	bool read(FileRef & file, uint32 length)
	{
		if(length != 8)
		{
			E_ASSERT(0); return false;
		}
		if(!nb_png_read_uint32(file, num_frames)
			|| !nb_png_read_uint32(file, num_loop)
			|| !nb_png_read_uint32(file, crc))
		{
			E_ASSERT(0); return false;
		}
		return true;
	}

	bool write(FileRef & file)
	{
		crc = 0xffffffff;
		nb_crc32(crc, 'a');
		nb_crc32(crc, 'c');
		nb_crc32(crc, 'T');
		nb_crc32(crc, 'L');
		nb_crc32(crc, (num_frames >> 24) & 0xff);
		nb_crc32(crc, (num_frames >> 16) & 0xff);
		nb_crc32(crc, (num_frames >> 8) & 0xff);
		nb_crc32(crc, num_frames  & 0xff);
		nb_crc32(crc, (num_loop >> 24) & 0xff);
		nb_crc32(crc, (num_loop >> 16) & 0xff);
		nb_crc32(crc, (num_loop >> 8) & 0xff);
		nb_crc32(crc, num_loop  & 0xff);
		crc = crc ^ 0xffffffff;

		if(!nb_png_write_uint32(file, 8)
			|| !file->Write("acTL", 4)
			|| !nb_png_write_uint32(file, num_frames)
			|| !nb_png_write_uint32(file, num_loop)
			|| !nb_png_write_uint32(file, crc))
		{
			E_ASSERT(0); return false;
		}

		return true;
	}

};

struct FCTL
{
//	uint32 sn;
	uint32 w;
	uint32 h;
	uint32 x;
	uint32 y;
	uint16 delay_num;
	uint16 delay_den;
	uint8  dispose;
	uint8  blend;
	FCTL()
	{
		//sn = -1;
		w = 0;
		h = 0;
		x = 0;
		y = 0;
		delay_num = 1;
		delay_den = 10;
		dispose   = 0;
		blend     = 0;
	}

	int get_delay_ms() const
	{
		if(delay_den)
		{
			return (int)delay_num * 1000 / (int)delay_den;
		}
		else
		{
			return (int)delay_num * 10;
		}
	}

	void set_delay_ms(int _ms)
	{
		delay_num = _ms;
		delay_den = 1000;
	}
	bool region_constract(uint32 header_w, uint32 header_h)
	{
		return w > 0 
			&& h > 0 
			&& x + w <= header_w 
			&& y + h <= header_h;
	}

	bool read(FileRef & file, uint32 length)
	{
		if(length != 26)
		{
			E_ASSERT(0); return false;
		}
		uint32 sn;
		if(!nb_png_read_uint32(file, sn)
			|| !nb_png_read_uint32(file, w)
			|| !nb_png_read_uint32(file, h)
			|| !nb_png_read_uint32(file, x)
			|| !nb_png_read_uint32(file, y)
			|| !nb_png_read_uint16(file, delay_num)
			|| !nb_png_read_uint16(file, delay_den)
			|| !file->Read(&dispose, 1)
			|| !file->Read(&blend, 1)
			|| !file->Skip(4))
		{
			E_ASSERT(0); return false;
		}
		return true;
	}

	bool write(FileRef & file, uint32 sn)
	{
		uint32 crc = 0xffffffff;
		nb_crc32(crc, 'f');
		nb_crc32(crc, 'c');
		nb_crc32(crc, 'T');
		nb_crc32(crc, 'L');

		nb_crc32(crc, (sn >> 24) & 0xff);
		nb_crc32(crc, (sn >> 16) & 0xff);
		nb_crc32(crc, (sn >> 8) & 0xff);
		nb_crc32(crc, sn  & 0xff);

		nb_crc32(crc, (w >> 24) & 0xff);
		nb_crc32(crc, (w >> 16) & 0xff);
		nb_crc32(crc, (w >> 8) & 0xff);
		nb_crc32(crc, w  & 0xff);

		nb_crc32(crc, (h >> 24) & 0xff);
		nb_crc32(crc, (h >> 16) & 0xff);
		nb_crc32(crc, (h >> 8) & 0xff);
		nb_crc32(crc, h  & 0xff);

		nb_crc32(crc, (x >> 24) & 0xff);
		nb_crc32(crc, (x >> 16) & 0xff);
		nb_crc32(crc, (x >> 8) & 0xff);
		nb_crc32(crc, x  & 0xff);

		nb_crc32(crc, (y >> 24) & 0xff);
		nb_crc32(crc, (y >> 16) & 0xff);
		nb_crc32(crc, (y >> 8) & 0xff);
		nb_crc32(crc, y  & 0xff);

		nb_crc32(crc, (delay_num >> 8) & 0xff);
		nb_crc32(crc, delay_num  & 0xff);

		nb_crc32(crc, (delay_den >> 8) & 0xff);
		nb_crc32(crc, delay_den  & 0xff);

		nb_crc32(crc, dispose);
		nb_crc32(crc, blend);

		crc = crc ^ 0xffffffff;

		if(!nb_png_write_uint32(file, 26)
			|| !file->Write("fcTL", 4)
			|| !nb_png_write_uint32(file, sn)
			|| !nb_png_write_uint32(file, w)
			|| !nb_png_write_uint32(file, h)
			|| !nb_png_write_uint32(file, x)
			|| !nb_png_write_uint32(file, y)
			|| !nb_png_write_uint16(file, delay_num)
			|| !nb_png_write_uint16(file, delay_den)
			|| !file->Write(&dispose, 1)
			|| !file->Write(&blend, 1)
			|| !nb_png_write_uint32(file, crc))
		{
			E_ASSERT(0); return false;
		}
		return true;
	}

	void read_ini(IniFile &_ini, const string & _ss)
	{
		int tmp;
		_ini.Get(_ss + L"x", (int&)(x));
		_ini.Get(_ss + L"y", (int&)(y));
		tmp = this->get_delay_ms();
		_ini.Get(_ss + L"delay", tmp);
		this->set_delay_ms(tmp);
		string s;
		_ini.Get(_ss + L"dispose", s);
		if(s.icompare(L"none") == 0)
		{
			dispose = APNG_DISPOSE_OP_NONE;
		}
		else if(s.icompare(L"background") == 0)
		{
			dispose = APNG_DISPOSE_OP_BACKGROUND;
		}
		else if(s.icompare(L"previous") == 0)
		{
			dispose = APNG_DISPOSE_OP_PREVIOUS;
		}

		s.clear();
		_ini.Get(_ss + L"blend", s);
		if(s.icompare(L"source") == 0)
		{
			blend = APNG_BLEND_OP_SOURCE;
		}
		else if(s.icompare(L"over") == 0)
		{
			blend = APNG_BLEND_OP_OVER;
		}

	}

	void write_ini(IniFile &_ini, const string & _ss)
	{
		_ini.Set(_ss + L"x", (int&)(x));
		_ini.Set(_ss + L"y", (int&)(y));
		_ini.Set(_ss + L"delay", this->get_delay_ms());
		switch(dispose)
		{
		case APNG_DISPOSE_OP_NONE:
			_ini.Set(_ss + L"dispose", L"none");
			break;
		default:
		case APNG_DISPOSE_OP_BACKGROUND:
			_ini.Set(_ss + L"dispose", L"background");
			break;
		case APNG_DISPOSE_OP_PREVIOUS:
			_ini.Set(_ss + L"dispose", L"previous");
			break;
		}

		switch(blend)
		{
		case APNG_BLEND_OP_SOURCE:
			_ini.Set(_ss + L"blend", L"source");
			break;
		default:
		case APNG_BLEND_OP_OVER:
			_ini.Set(_ss + L"blend", L"over");
			break;
		}
	}

};

struct IDAT
{
//	uint32 sn;
	uint32  length;
	uint8 * data;
	IDAT()
	{
		data = 0;
	}
	~IDAT()
	{
		free(data);
	}
	bool read(FileRef & file, uint32 _len, bool _fdat)
	{
		//if(!nb_png_read_uint32(file, length))
		//{
		//	E_ASSERT(0); return false;
		//}
		length = _len;
		if(_fdat)
		{
			uint32 sn;
			if(!nb_png_read_uint32(file, sn))
			{
				E_ASSERT(0); return false;
			}
			length-=4;
		}
		data = (uint8*) realloc(data, length);
		if(!file->Read(data, length)
			|| !file->Skip(4)) // crc
		{
			free(data);
			data = 0;
			E_ASSERT(0); return false;
		}
		return true;
	}

	bool write(FileRef & file, bool _fdat, uint32 sn)
	{
		if(data == 0)
		{
			E_ASSERT(0); return false;
		}

		if(_fdat)
		{
			uint32 crc = 0xffffffff;
			nb_crc32(crc, 'f');
			nb_crc32(crc, 'd');
			nb_crc32(crc, 'A');
			nb_crc32(crc, 'T');
			nb_crc32(crc, (sn >> 24) & 0xff);
			nb_crc32(crc, (sn >> 16) & 0xff);
			nb_crc32(crc, (sn >> 8) & 0xff);
			nb_crc32(crc, sn  & 0xff);
			for(int i=0; i<length; i++)
			{
				nb_crc32(crc, data[i]);
			}
			crc = crc ^ 0xffffffff;

			if(!nb_png_write_uint32(file, length+4)
				|| !file->Write("fdAT", 4)
				|| !nb_png_write_uint32(file, sn)
				|| !file->Write(data, length)
				|| !nb_png_write_uint32(file, crc))
			{
				E_ASSERT(0); return false;
			}

		}
		else
		{
			uint32 crc = 0xffffffff;
			nb_crc32(crc, 'I');
			nb_crc32(crc, 'D');
			nb_crc32(crc, 'A');
			nb_crc32(crc, 'T');
			for(int i=0; i<length; i++)
			{
				nb_crc32(crc, data[i]);
			}
			crc = crc ^ 0xffffffff;

			if(!nb_png_write_uint32(file, length)
				|| !file->Write("IDAT", 4)
				|| !file->Write(data, length)
				|| !nb_png_write_uint32(file, crc))
			{
				E_ASSERT(0); return false;
			}
		}
		return true;
	}
};

static bool write_iend(FileRef & file)
{
	uint32 iend_crc32 = 0xffffffff;
	nb_crc32(iend_crc32, 'I');
	nb_crc32(iend_crc32, 'E');
	nb_crc32(iend_crc32, 'N');
	nb_crc32(iend_crc32, 'D');
	iend_crc32 = iend_crc32 ^ 0xffffffff;

	if( !nb_png_write_uint32(file, 0)
		|| !file->Write("IEND", 4)
		|| !nb_png_write_uint32(file, iend_crc32)
		)
	{
		E_ASSERT(0);
		E_ASSERT(0); return false;
	}
	return true;
}

struct ANY_CHUNK
{
	uint32 length;
	uint32 name;
	uint8 * data;
	uint32 crc;
	ANY_CHUNK()
	{
		data = 0;
	}
	~ANY_CHUNK()
	{
		free(data);
	}

	bool read(FileRef & file, uint32 _length, uint32 _name)
	{
		length = _length;
		name = _name;
		data = (uint8*) realloc(data, length);
		if(!file->Read(data, length)
			|| !nb_png_read_uint32(file, crc))
		{
			free(data);
			data = 0;
			E_ASSERT(0); return false;
		}
		return true;
	}

	bool write(FileRef & file)
	{
		if(!nb_png_write_uint32(file, length)
			|| !file->Write(&name, 4)
			|| !file->Write(data, length)
			|| !nb_png_write_uint32(file, crc))
		{
			E_ASSERT(0); return false;
		}
		return true;
	}
};

struct FRAME
{
	IHDR ihdr;
	FCTL fctl;
	IDAT fdat;
	Array<ANY_CHUNK*> addition_chunks;

	void delete_addition_chunks()
	{
		for(int i=0; i<addition_chunks.size(); i++)
		{
			delete addition_chunks[i];
		}
		addition_chunks.clear();
	}
	~FRAME()
	{
		delete_addition_chunks();
	}

	bool load(const Path & _path)
	{
		delete_addition_chunks();
		FileRef file = FS::OpenFile(_path);
		if(!file)
		{
			message(L"[nb] (WW) Failed to load png: " + _path.GetBaseName(true));
			E_ASSERT(0); return false;
		}

		if(!check_signature(file))
		{
			E_ASSERT(0); return false;
		}

		if(!ihdr.read(file))
		{
			E_ASSERT(0); return false;
		}

		fctl.w = ihdr.width;
		fctl.h = ihdr.height;

		bool end = false;
		do
		{
			uint32 chunk_length;
			if(!nb_png_read_uint32(file, chunk_length))
			{
				E_ASSERT(0);
				E_ASSERT(0); return false;
			}

			uint32 chunk_type;
			if(!file->Read(&chunk_type, 4))
			{
				E_ASSERT(0);
				E_ASSERT(0); return false;
			}

			{
				string chunk_name1 = "ABCD";
				for(int i=0; i<4; i++)
				{
					chunk_name1[i] = (chunk_type >> (i*8)) & 0xff;
				}

				E_TRACE_LINE(L"[nb] chunk: " + QuoteString(chunk_name1) + L"  len=" + string(chunk_length));
			}

			if(0x444E4549 == chunk_type)
			{
				E_ASSERT(0); return false;
			}
			if(chunk_length == 0)
			{
				file->Skip(4);
				continue;
			}

			switch(chunk_type)
			{
			case 0x54414449: // IDAT
				if(!fdat.read(file, chunk_length, false))
				{
					E_ASSERT(0); return false;
				}
				return true;

			default:
				{
					ANY_CHUNK * p = enew ANY_CHUNK();
					if(!p->read(file, chunk_length, chunk_type))
					{
						delete p;
						E_ASSERT(0); return false;
					}
					addition_chunks.push_back(p);
				}
				break;
			}
		}while(!end);

		return true;
	}

	bool save(const Path & _path)
	{
		FileRef file = FS::OpenFile(_path, true);
		if(!file)
		{
			E_ASSERT(0); return false;
		}

		file->SetSize(0);

		if(!write_signature(file))
		{
			E_ASSERT(0); return false;
		}

		if(!ihdr.write(file))
		{
			E_ASSERT(0); return false;
		}


		for(int i=0; i<addition_chunks.size(); i++)
		{
			if(!addition_chunks[i]->write(file))
			{
				E_ASSERT(0); return false;
			}
		}

		if(!fdat.write(file, false, 0))
		{
			E_ASSERT(0); return false;
		}

		if(!write_iend(file))
		{
			E_ASSERT(0); return false;
		}

		return true;
	}
};

static bool unpack_apng(IniFile & _ini, const Path & _path, const Path & _out_folder)
{
	IHDR ihdr;
	ACTL actl;
	FRAME f;

	FileRef file = FS::OpenFile(_path);
	if(!file)
	{
		message(L"[nb] (WW) Failed to load png: " + _path.GetBaseName(true));
		E_ASSERT(0); return false;
	}

	if(!check_signature(file))
	{
		E_ASSERT(0); return false;
	}

	if(!ihdr.read(file))
	{
		E_ASSERT(0); return false;
	}
	bool has_actl = false;
	int fctl_count = 0;
	int idat_count = 0;
	bool end = false;
	do
	{
		uint32 chunk_length;
		if(!nb_png_read_uint32(file, chunk_length))
		{
			E_ASSERT(0);
			E_ASSERT(0); return false;
		}

		uint32 chunk_type;
		if(!file->Read(&chunk_type, 4))
		{
			E_ASSERT(0);
			E_ASSERT(0); return false;
		}

		{
			string chunk_name1 = "ABCD";
			for(int i=0; i<4; i++)
			{
				chunk_name1[i] = (chunk_type >> (i*8)) & 0xff;
			}

			E_TRACE_LINE(L"[nb] chunk: " + QuoteString(chunk_name1) + L"  len=" + string(chunk_length));
		}

		if(0x444E4549 == chunk_type)
		{
			return idat_count > 0 && idat_count == fctl_count;
		}

		if(chunk_length == 0)
		{
			file->Skip(4);
			continue;
		}

		switch(chunk_type)
		{
		case 0x4C546361: // acTL
			if(has_actl)
			{
				E_ASSERT(0); return false;
			}
			if(!actl.read(file, chunk_length))
			{
				E_ASSERT(0); return false;
			}
			has_actl = true;
			_ini.Set(L"[main]frames", (int&)actl.num_frames);
			_ini.Set(L"[main]loop", (int&)actl.num_loop);
			break;

		case 0x4C546366: // fcTL
			fctl_count++;
			if(!has_actl)
			{
				E_ASSERT(0); return false;
			}
			if(!f.fctl.read(file, chunk_length))
			{
				E_ASSERT(0); return false;
			}
			break;

		case 0x54414449: // IDAT
		case 0x54416466: // fDAT
			if(!has_actl)
			{
				E_ASSERT(0); return false;
			}
			if(idat_count+1 != fctl_count)
			{
				E_ASSERT(0); return false;
			}
			idat_count++;
			if(!f.fdat.read(file, chunk_length, 0x54416466 == chunk_type))
			{
				E_ASSERT(0); return false;
			}
			{
				string ss = L"[" + string(idat_count-1) + L"]";
				f.fctl.write_ini(_ini, ss);
				string pngName = _path.GetBaseName(false) + L"/" + _path.GetBaseName(false) + L"-" + string(idat_count-1) + L".png";
				_ini.Set(ss + L"file", pngName);
				Path pngPath = _out_folder | pngName;
				f.ihdr = ihdr;
				f.ihdr.width  = f.fctl.w;
				f.ihdr.height = f.fctl.h;
				if(!f.save(pngPath))
				{
					E_ASSERT(0);
					message(L"[apng] (WW) frame save failed.");
				}
				else
				{
					//Image image;
					//image.Load(pngPath);
				}
			}
			break;

		default:
			if(fctl_count == 0)
			{
				ANY_CHUNK * p = enew ANY_CHUNK();
				if(!p->read(file, chunk_length, chunk_type))
				{
					delete p;
					E_ASSERT(0); return false;
				}
				f.addition_chunks.push_back(p);
			}
			else 
			{
				if(!file->Skip(chunk_length+4))
				{
					E_ASSERT(0); return false;
				}
			}
			break;
		}
	}while(!end);

	return true;
}

static bool unpack_apng(const Path & _path)
{
	Path out_folder = _path.GetParentFolder() | L"output-unpack";
	Path iniPath = out_folder | _path.GetBaseName(false) + L".ini";
	FS::Delete(iniPath);
	IniFile iniFile;
	if(unpack_apng(iniFile, _path, out_folder))
	{
		return iniFile.Save(iniPath);
	}
	else
	{
		return false;
	}
}

static bool pack_apng(FileRef & file, const Path & _path)
{
	IHDR ihdr;
	ACTL actl;
	FRAME f;

	IniFile iniFile;
	iniFile.Load(_path);
	iniFile.Get(L"[main]frames", (int&)actl.num_frames);
	iniFile.Get(L"[main]loop", (int&)actl.num_loop);

	if(actl.num_frames == 0)
	{
		E_ASSERT(0); return false;
	}
	if(!write_signature(file))
	{
		E_ASSERT(0); return false;
	}

	int sn = 0;
	string src_png_file;
	for(int i=0; i<actl.num_frames; i++)
	{
		string ss = L"[" + string(i) + L"]";
		iniFile.GetString(ss + L"file", src_png_file);
		Path path = src_png_file;
		if(path.IsRelative())
		{
			path = _path.GetParentFolder() | src_png_file;
		}
		if(!f.load(path))
		{
			E_ASSERT(0); return false;
		}
		f.fctl.read_ini(iniFile, ss);
		if(i == 0)
		{
			ihdr = f.ihdr;
			if(!ihdr.write(file))
			{
				E_ASSERT(0); return false;
			}
			for(int i=0; i<f.addition_chunks.size(); i++)
			{
				if(!f.addition_chunks[i]->write(file))
				{
					E_ASSERT(0); return false;
				}
			}
			f.delete_addition_chunks();
			if(!actl.write(file)
				|| !f.fctl.write(file, sn++)
				|| !f.fdat.write(file, false, 0))
			{
				E_ASSERT(0); return false;
			}
		}
		else
		{
			if(!check_ihdr_compatible(ihdr, f.ihdr))
			{
				E_ASSERT(0); return false;
			}
			f.delete_addition_chunks();
			if(!f.fctl.write(file, sn++)
				|| !f.fdat.write(file, true, sn++))
			{
				E_ASSERT(0); return false;
			}
		}
	}

	write_iend(file);

	return true;
}


static bool pack_apng(const Path & _path)
{
	Path out_folder = _path.GetParentFolder() | L"output-apng";
	Path out_apng_path = out_folder | _path.GetBaseName(false) + L".png";
	FileRef file = FS::OpenFile(out_apng_path, true);
	if(!file)
	{
		E_ASSERT(0); return false;
	}
	file->SetSize(0);
	if(!pack_apng(file, _path))
	{
		file = 0;
		FS::Delete(out_apng_path);
	}
	else
	{
		//Image image;
		//image.Load(path1);
	}
	return true;
}

int main(int _argc, char ** _argv)
{
	string fileName;
	if(_argc >= 2)
	{
		fileName = stringa(_argv[1]);
	}
	else
	{
#ifdef NB_WINDOWS
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH];
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = L'\0';
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFilter = L"Supported files\0*.png;*.apng;*.ini\0All files\0*.*\0\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		if(GetOpenFileName(&ofn)==TRUE)
		{
			fileName = ofn.lpstrFile;
		}
#endif
	}

	if(fileName.empty())
	{
		printf("usage: \n");
		printf("apng <file-name>");
		return 1;
	}

	Path path = fileName;

	if(!FS::IsFile(path))
	{
		printf("failed to open file.\n");
		return 2;
	}
	fileName = path.GetBaseName();
	if(MatchFileName(fileName, L"*.png") || MatchFileName(fileName, L"*.apng"))
	{
		if(!unpack_apng(path))
		{
			return 3;
		}
	}
	else
	{
		if(!pack_apng(path))
		{
			return 3;
		}
	}

	return 0;
}

#pragma pack(pop)

