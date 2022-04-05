#include <string.h>
// #include <xmmintrin.h>
#include <nbug/core/path.h>
#include <nbug/core/file.h>
#include <nbug/core/ref.h>
#include <nbug/core/env.h>
#include <nbug/core/zip.h>
#include <nbug/gl/image.h>

#define NB_PNG_DEBUG_TRACE
#define NB_PNG_SAVE_SOFTWARE_NAME
#pragma warning(disable : 4996)

namespace e
{
	extern uint64 low_bits_mask_64[65];
	typedef uint8 Pixel[4];

	enum
	{
		PNG_COLOR_TYPE_L     = 0,
		PNG_COLOR_TYPE_RGB   = 2,
		PNG_COLOR_TYPE_INDEX = 3,
		PNG_COLOR_TYPE_LA    = 4,
		PNG_COLOR_TYPE_RGBA  = 6,
	};

	struct NB_PNG_INTERLACE_INFO
	{
		int x0;
		int dx;
		int y0;
		int dy;
	};

	static NB_PNG_INTERLACE_INFO g_interlace_info[7] =
	{
		{0, 8, 0, 8},
		{4, 8, 0, 8},
		{0, 4, 4, 8},
		{2, 4, 0, 4},
		{0, 2, 2, 4},
		{1, 2, 0, 2},
		{0, 1, 1, 2},
	};

	typedef uint8 NB_PNG_PALETE[4];

	struct IMG_BLOCK
	{
		uint8* buf;
		uint32 buf_size;
		uint32 w; // in pixel
		uint32 h; // in pixel
		uint32 channel;
		uint32 bit_depth;
		uint32 fu;
		uint32 src_line_size;
		uint32 dst_line_size;
		uint32 ex_dst_line_size; // fu + dst_line_size
		//uint32 len;
		int ppb; // pixel per byte
		uint8 bit_mask; // = (0x01 << bit_depth) - 1;
		int bit_to_byte_shift;
		int bit_to_byte_mask;

		IMG_BLOCK()
		{ buf = 0; }

		~IMG_BLOCK()
		{ free(buf); }

		void alloc(uint32 _w, uint32 _h, int _channel, uint8 _bit_depth)
		{
			w = _w;
			h = _h;
			channel = _channel;
			bit_depth = _bit_depth;
			if(w && h)
			{
				fu = bit_depth >= 8 ? bit_depth/8 * channel : 1;
				dst_line_size = bit_depth >= 8 ? fu * w : (bit_depth * w + 0x07) >> 3;
				src_line_size = dst_line_size + 1;
				ex_dst_line_size = dst_line_size + fu;
				buf_size = ex_dst_line_size * (h + 1);
				buf = (uint8*) malloc(buf_size);
				ppb = 8 / bit_depth;
				bit_mask = (0x01 << bit_depth) - 1;
				switch(bit_depth)
				{
				case 1:
					bit_to_byte_shift = 3;
					bit_to_byte_mask  = 0x07;
					break;
				case 2:
					bit_to_byte_shift = 2;
					bit_to_byte_mask  = 0x03;
					break;
				case 4:
					bit_to_byte_shift = 1;
					bit_to_byte_mask  = 0x01;
					break;
				}
			}
		}
		uint8 * get_line(int _y) const
		{ return buf + (_y + 1) * ex_dst_line_size + fu; }

		uint8 * get_ex_line(int _y) const
		{ return buf + (_y + 1) * ex_dst_line_size; }

		// get and set func are mainly for rare used interlacing and 1,2,4 bit depth,
		// we just load them properly, but don't concern about performance.
		void get(uint32 _x, uint32 _y, Pixel & _pixel) const
		{
			if(bit_depth == 8)
			{
				uint8 * p = get_line(_y) + _x * channel;;
				for(int i=0; i<channel; i++)
				{
					_pixel[i] = p[i];
				}
			}
			else
			{
				uint8 * line = get_line(_y);
				uint8 * p = line + (_x >> bit_to_byte_shift);
				int bi_idx = ppb - (_x & bit_to_byte_mask)-1;

				for(int i=0; i<channel; i++)
				{
					_pixel[i] = ((*p) >> (bi_idx * bit_depth)) & bit_mask;
					if(bi_idx == 0)
					{
						bi_idx = ppb - 1;
						p++;
					}
					else
					{
						bi_idx--;
					}
				}
			}
		}

		void set(uint32 _x, uint32 _y, const Pixel & _pixel)
		{
			if(bit_depth == 8)
			{
				uint8 * p = get_line(_y) + _x * channel;;
				for(int i=0; i<channel; i++)
				{
					 p[i] = _pixel[i];
				}
			}
			else
			{
				uint8 * line = get_line(_y);
				uint8 * p = line + (_x >> bit_to_byte_shift);
				int bi_idx = ppb - (_x & bit_to_byte_mask)-1;

				for(int i=0; i<channel; i++)
				{
					//_pixel[i] = ((*p) >> (bi_idx * ppb)) & bit_mask;
					(*p)= ((_pixel[i] & bit_mask) << (bi_idx * bit_depth)) | (~(bit_mask << (bi_idx * bit_depth)) & (*p));
					if(bi_idx == 0)
					{
						bi_idx = ppb - 1;
						p++;
					}
					else
					{
						bi_idx--;
					}
				}
			}
		}

		bool filter(uint8 * _src, size_t _src_len)
		{
			if(src_line_size * h != _src_len)
			{
				E_ASSERT(0);
				return false;
			}

			memset(buf, 0, ex_dst_line_size);
			uint8 * pxs = _src;
			uint8 * pxd = buf + ex_dst_line_size;
			uint8 * pa;
			uint8 * pb;
			uint8 * pc;

			for(uint32 ln=0; ln<h; ln++)
			{
				//E_ASSERT(pxd < buf + buf_size);
				E_ASSERT(pxd == get_ex_line(ln));
				uint8 * end = pxs + src_line_size;
				uint8 filter = *pxs++;
				for(int i=0; i<fu; i++)
				{
					*pxd++ = 0;
				}
				switch(filter)
				{
				case 0:
					memcpy(pxd, pxs, dst_line_size);
					pxd+= dst_line_size;
					pxs+= dst_line_size;
					break;
				case 1:
					pa = pxd - fu;
					while(pxs!=end)
					{
						*pxd++ = (*pxs++) + (*pa++);
					}
					break;
				case 2:
					pb = pxd - ex_dst_line_size;
					while(pxs!=end)
					{
						*pxd++ = (*pxs++) + (*pb++);
					}
					break;
				case 3:
					pa = pxd - fu;
					pb = pxd - ex_dst_line_size;
					while(pxs!=end)
					{
						*pxd++ = (*pxs++) + uint8((int(*pa++) + int(*pb++)) >> 1);
					}
					break;
				case 4:
					pa = pxd - fu;
					pb = pxd - ex_dst_line_size;
					pc = pb - fu;
					while(pxs!=end)
					{
						int aaa = *pa++;
						int bbb = *pb++;
						int ccc = *pc++;
						int p = aaa + bbb - ccc;
						int p_a = p - aaa;
						int p_b = p - bbb;
						int p_c = p - ccc;
						int pa = p_a < 0 ? -p_a : p_a;
						int pb = p_b < 0 ? -p_b : p_b;
						int pc = p_c < 0 ? -p_c : p_c;
						int pr = (pa <= pb && pa <= pc) ? aaa : ((pb <= pc ) ? bbb : ccc);
						*pxd++ = (*pxs++) + pr;
					}
					break;
				default:
					E_ASSERT(0);
					return false;
				}
			}
			return true;
		}

	};

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
			return false;
		}
	}

	inline static bool nb_png_read_uint32(FileRef & file, uint32 & _n)
	{
		uint8 buf[4];
		if(file->Read(buf, 4))
		{
			_n = uint32(buf[0]) << 24 | uint32(buf[1]) << 16 | uint32(buf[2]) << 8 | uint32(buf[3]);
			return true;
		}
		else
		{
			return false;
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

	struct NB_PNG_DATA
	{
		// header
		uint32 w;
		uint32 h;
		uint8  bit_depth;
		uint8  color_type;
		uint8  compression_method;
		uint8  filter_method;
		uint8  interlace_method;
		int channel;

		// other
		uint8 * data;
		size_t data_len;
		NB_PNG_PALETE * palete;
		uint32 palete_size;
		bool has_transparent_color;
		uint8 transparent_color[4];
		Image result;

		NB_PNG_DATA()
		{
			data = 0;
			data_len = 0;
			palete = 0;
			palete_size = 0;
			has_transparent_color = false;
			channel = 0;
		}

		~NB_PNG_DATA()
		{
			free(data);
			free(palete);
		}

#ifdef NB_DEBUG
		const char * get_color_type_name()
		{
			switch(color_type)
			{
			case PNG_COLOR_TYPE_L:
				return "Greyscale";
			case PNG_COLOR_TYPE_RGB:
				return "Truecolour RGB";
			case PNG_COLOR_TYPE_INDEX:
				return "Indexed-colour";
			case PNG_COLOR_TYPE_LA:
				return "Greyscale with alpha";
			case PNG_COLOR_TYPE_RGBA:
				return "Truecolour with alpha RGBA";
			default:
				return "Unkown";
			}
		}
#endif

		bool read_header(FileRef file)
		{
#ifdef NB_PNG_DEBUG_TRACE
			E_TRACE_LINE(L"[nb] chunk: IHDR");
#endif
			uint32 chunk_length;
			if(!nb_png_read_uint32(file, chunk_length) || chunk_length != 13)
			{
				E_ASSERT(0);
				return false;
			}

			uint8 buf[5];
			// first chunk must be IHDR
			if(!file->Read(buf, 4) || memcmp(buf, "IHDR", 4) != 0)
			{
				E_ASSERT(0);
				return false;
			}

			if(!nb_png_read_uint32(file, this->w)
				|| !nb_png_read_uint32(file, this->h)
				|| !file->Read(buf, 5))
			{
				E_ASSERT(0);
				return false;
			}

			this->bit_depth = buf[0];
			this->color_type = buf[1];
			this->compression_method = buf[2];
			this->filter_method = buf[3];
			this->interlace_method = buf[4];

#ifdef NB_PNG_DEBUG_TRACE
			E_TRACE_LINE(L"[nb]   width = " + string(this->w) + L", height = " + string(this->h));
			E_TRACE_LINE(L"[nb]   bit_depth = " + string(this->bit_depth));
			E_TRACE_LINE(L"[nb]   color_type = " + string(this->color_type) + L" (" +string(get_color_type_name()) +L")");
			E_TRACE_LINE(L"[nb]   compression_method = " + string(this->compression_method));
			E_TRACE_LINE(L"[nb]   filter_method = " + string(this->filter_method));
			E_TRACE_LINE(L"[nb]   interlace_method = " + string(this->interlace_method));
#endif

			if(!this->is_header_valid())
			{
				E_ASSERT(0);
				return false;
			}

			int8 _table[] = {1, 0, 3, 1, 2, 0, 4};
			this->channel = _table[this->color_type];

			uint32 crc32;
			if(!file->Read(&crc32, 4))
			{
				E_ASSERT(0);
				return false;
			}

			// TODO: check crc

			return true;
		}

		bool is_header_valid() const
		{
			if(w < 1 || h < 1)
			{
				E_ASSERT(0);
				return false;
			}

			if(compression_method != 0 || filter_method != 0 || interlace_method > 1)
			{
				E_ASSERT(0);
				return false;
			}

			switch(color_type)
			{
			case PNG_COLOR_TYPE_L:
				if(bit_depth != 1 && bit_depth != 2 && bit_depth!=4 && bit_depth != 8 && bit_depth != 16)
				{
					E_ASSERT(0);
					return false;
				}
				break;
			case PNG_COLOR_TYPE_RGB:
			case PNG_COLOR_TYPE_LA:
			case PNG_COLOR_TYPE_RGBA:
				if(bit_depth != 8 && bit_depth != 16)
				{
					E_ASSERT(0);
					return false;
				}
				break;
			case PNG_COLOR_TYPE_INDEX:
				if(bit_depth != 1 && bit_depth != 2 && bit_depth!=4 && bit_depth != 8)
				{
					E_ASSERT(0);
					return false;
				}
				break;
			default:
				E_ASSERT(0);
				return false;
			}

			return true;
		}

		bool interlacing_filter(IMG_BLOCK & _decoded)
		{
			_decoded.alloc(w, h, channel, bit_depth);

			if(interlace_method == 1)
			{
				size_t offset = 0;
				for(int i=0; i<7; i++)
				{
					NB_PNG_INTERLACE_INFO & info = g_interlace_info[i];
					IMG_BLOCK reduced;
					uint32 w1 = (w - info.x0 - 1) / info.dx + 1;
					uint32 h1 = (h - info.y0 - 1) / info.dy + 1;
					if(info.x0 < w && info.y0 <h
						&& w1 && h1)
					{
						reduced.alloc(w1, h1, channel, bit_depth);
					}
					else
					{
						continue; // empty pass
					}
					uint8 * src = data + offset;
					size_t len = reduced.src_line_size * reduced.h;
					if(offset + len > data_len)
					{
						E_ASSERT(0);
						return false;
					}
					if(!reduced.filter(src, len))
					{
						E_ASSERT(0);
						return false;
					}
					offset+=len;

					Pixel tmp;
					int ix = info.x0;
					for(int x=0; x<reduced.w; x++,ix+=info.dx)
					{
						int iy = info.y0;
						for(int y=0; y<reduced.h; y++,iy+=info.dy)
						{
							reduced.get(x, y, tmp);
							_decoded.set(ix, iy, tmp);
						}
					}
				}

			}
			else
			{
				if(!_decoded.filter(data, data_len))
				{
					E_ASSERT(0);
					return false;
				}
			}

			return true;
		}

		bool decode_8()
		{
			IMG_BLOCK decoded;
			if(!interlacing_filter(decoded))
			{
				return false;
			}

			uint8 * src = decoded.buf+decoded.ex_dst_line_size + decoded.fu;
			result.Alloc(w, h);
			uint8 * dst = result.data;
			int fu = decoded.fu;

			switch(color_type)
			{
			case PNG_COLOR_TYPE_L:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						uint8 l = *src++;
						*dst++ = l;
						*dst++ = l;
						*dst++ = l;
						*dst++ = 0xff;
					}
					src+=fu;
				}
				break;
			case PNG_COLOR_TYPE_LA:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						uint8 l = *src++;
						*dst++ = l;
						*dst++ = l;
						*dst++ = l;
						*dst++ = *src++;
					}
					src+=fu;
				}
				break;
			case PNG_COLOR_TYPE_RGB:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						*dst++ = *src++;
						*dst++ = *src++;
						*dst++ = *src++;
						*dst++ = 0xff;
					}
					src+=fu;
				}
				break;
			case PNG_COLOR_TYPE_RGBA:
				for(int y=0; y<h; y++)
				{
					memcpy(dst, src, w*4);
					src+= decoded.ex_dst_line_size;
					dst+= w * 4;
				}
				break;
			case PNG_COLOR_TYPE_INDEX:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						uint8 idx = *src++;
						*dst++ = palete[idx][0];
						*dst++ = palete[idx][1];
						*dst++ = palete[idx][2];
						*dst++ = palete[idx][3];
					}
					src+=fu;
				}
				break;
			default:
				E_ASSERT(0);
				return false;
			}

			return true;
		}

		bool decode_1_2_4()
		{
			IMG_BLOCK decoded;
			if(!interlacing_filter(decoded))
			{
				return false;
			}

			result.Alloc(w, h);
			uint8 * dst = result.data;

			uint8 mask = (0x01 << bit_depth) - 1;
			uint8 multi = (256 / mask) - 1;
			Pixel tmp;
			switch(color_type)
			{
			case PNG_COLOR_TYPE_L:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						decoded.get(x, y, tmp);
						uint8 l = tmp[0] * multi;
						*dst++ = l;	*dst++ = l; *dst++ = l;	*dst++ = 0xff;
					}
				}
				break;
			case PNG_COLOR_TYPE_LA:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						decoded.get(x, y, tmp);
						uint8 l = tmp[0] * multi;
						uint8 a = tmp[1] * multi;
						*dst++ = l;	*dst++ = l; *dst++ = l;	*dst++ = a;
					}
				}
				break;
			case PNG_COLOR_TYPE_RGB:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						decoded.get(x, y, tmp);
						uint8 r = tmp[0] * multi;
						uint8 g = tmp[1] * multi;
						uint8 b = tmp[2] * multi;
						*dst++ = r;	*dst++ = g;	*dst++ = b;	*dst++ = 0xff;
					}
				}
				break;
			case PNG_COLOR_TYPE_RGBA:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						decoded.get(x, y, tmp);
						uint8 r = tmp[0] * multi;
						uint8 g = tmp[1] * multi;
						uint8 b = tmp[2] * multi;
						uint8 a = tmp[3] * multi;
						*dst++ = r;	*dst++ = g;	*dst++ = b;	*dst++ = a;
					}
				}
				break;
			case PNG_COLOR_TYPE_INDEX:
				for(int y=0; y<h; y++)
				{
					for(int x=0; x<w; x++)
					{
						decoded.get(x, y, tmp);
						uint8 idx = tmp[0];
						NB_PNG_PALETE & plt = palete[idx];
						*dst++ = plt[0]; *dst++ = plt[1]; *dst++ = plt[2]; *dst++ = plt[3];
					}
				}
				break;
			default:
				E_ASSERT(0);
				return false;
			};
			return true;
		}
	};


	static uint8 nb_png_sig[] = {137, 80, 78, 71, 13, 10, 26, 10};

	static bool LoadPng(Image & _pic, const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
#ifdef NB_PNG_DEBUG_TRACE
		E_TRACE_LINE(L"[nb] load png: " + _path.GetString());
#endif

		E_ASSERT(_pic.data == 0);
		FileRef file = FS::OpenFile(_path);
		if(!file)
		{
			message(L"[nb] (WW) Failed to load png: " + _path.GetBaseName(true));
			return false;
		}

		static uint8 buf[8];

		if(!file->Read(buf, 8) || memcmp(buf, nb_png_sig, 8) != 0)
		{
			E_ASSERT(0);
			return false;
		}

		NB_PNG_DATA png;
		if(!png.read_header(file))
		{
			E_ASSERT(0);
			return false;
		}


			uint32 crc32;
		bool end = false;
		do
		{
			uint32 chunk_length;
			if(!nb_png_read_uint32(file, chunk_length))
			{
				E_ASSERT(0);
				return false;
			}

			uint32 chunk_type;
			if(!file->Read(&chunk_type, 4))
			{
				E_ASSERT(0);
				return false;
			}
#ifdef NB_PNG_DEBUG_TRACE
			{
				string chunk_name1 = "ABCD";
				for(int i=0; i<4; i++)
				{
					chunk_name1[i] = (chunk_type >> (i*8)) & 0xff;
				}

				E_TRACE_LINE(L"[nb] chunk: " + QuoteString(chunk_name1) + L"  len=" + string(chunk_length));
			}
#endif

			switch(chunk_type)
			{
			case 0x45544C50: // PLTE
				if(png.palete != 0)
				{
					E_ASSERT(0);
					return false;
				}

				if(png.color_type != PNG_COLOR_TYPE_INDEX)
				{
					// only index-colour image need palete
					if(!file->Skip(chunk_length))
					{
						E_ASSERT(0);
						return false;
					}
					break;
				}

				if(chunk_length % 3 != 0 || chunk_length > 256*3)
				{
					E_ASSERT(0);
					return false;
				}

				{
					png.palete_size = chunk_length / 3;
					png.palete = (NB_PNG_PALETE*) malloc(256*4);
					memset(png.palete, 0xff, 256*4);
					for(int i=0; i<png.palete_size; i++)
					{
						if(!file->Read(png.palete[i], 3))
						{
							E_ASSERT(0);
							return false;
						}
					}
				}
				break;

			case 0x534e5274: // tRNS
				if(png.color_type == PNG_COLOR_TYPE_INDEX 
					&& png.palete && chunk_length<=png.palete_size)
				{
					for(int i=0; i<chunk_length; i++)
					{
						if(!file->Read(&png.palete[i][3], 1))
						{
							E_ASSERT(0);
							return false;
						}
					}
				}
				else if(png.color_type == PNG_COLOR_TYPE_RGB && chunk_length == 6)
				{
					png.has_transparent_color = true;
					uint16 v[3];
					if(!nb_png_read_uint16(file, v[0])
						|| !nb_png_read_uint16(file, v[1])
						|| !nb_png_read_uint16(file, v[2])
						)
					{
						E_ASSERT(0);
						return false;
					}

					png.transparent_color[0] = (uint8)(v[0] & low_bits_mask_64[png.bit_depth]);
					png.transparent_color[1] = (uint8)(v[1] & low_bits_mask_64[png.bit_depth]);
					png.transparent_color[2] = (uint8)(v[2] & low_bits_mask_64[png.bit_depth]);
					png.transparent_color[3] = 255;
					if(png.bit_depth < 8)
					{
						uint8 mask = (0x01 << png.bit_depth) - 1;
						uint8 multi = (256 / mask) - 1;
						png.transparent_color[0]*= multi;
						png.transparent_color[1]*= multi;
						png.transparent_color[2]*= multi;
					}
				}
				else if(png.color_type == PNG_COLOR_TYPE_L && chunk_length == 2)
				{
					png.has_transparent_color = true;
					uint16 v;
					if(!nb_png_read_uint16(file, v))
					{
						E_ASSERT(0);
						return false;
					}
					png.transparent_color[0] = (uint8)(v & low_bits_mask_64[png.bit_depth]);
					png.transparent_color[1] = png.transparent_color[0];
					png.transparent_color[2] = png.transparent_color[0];
					png.transparent_color[3] = 255;
					if(png.bit_depth < 8)
					{
						uint8 mask = (0x01 << png.bit_depth) - 1;
						uint8 multi = (256 / mask) - 1;
						png.transparent_color[0]*= multi;
						png.transparent_color[1]*= multi;
						png.transparent_color[2]*= multi;
					}
				}
				else
				{
					message(L"[nb] (WW) LoadPng(): irregular tRNS chunk.");
					if(!file->Skip(chunk_length))
					{
						E_ASSERT(0);
						return false;
					}
				}
				break;

			case 0x54414449: // IDAT
				if(chunk_length)
				{
					png.data = (uint8*) realloc(png.data, png.data_len + chunk_length);

					if(!file->Read(png.data+png.data_len, chunk_length))
					{
						E_ASSERT(0);
						return false;
					}

					png.data_len+= chunk_length;
				}
				break;

#if defined(NB_DEBUG) && defined(NB_PNG_DEBUG_TRACE)
			case 0x74584574: // tEXt
				{
					stringa keyword;
					keyword.reserve(chunk_length);
					if(!file->Read(&keyword[0], chunk_length))
					{
						E_ASSERT(0);
						return false;
					}
					keyword[chunk_length] = 0;
					stringa value;
					if(keyword.length() < chunk_length - 1)
					{
						value = &keyword[keyword.length()+2];
					}
					E_TRACE_LINE(L"[nb]   " + keyword + L" : " + value);
				}
				break;
#endif

			case 0x444E4549: // IEND
				end = true;
				break;


			default:
#ifdef NB_PNG_DEBUG_TRACE
				{
					uint8 tmp;
					for(int i=0; i<chunk_length; i++)
					{
						if(!file->Read(&tmp, 1))
						{
							E_ASSERT(0);
							return false;
						}
					}
				}
#else
				if(!file->Skip(chunk_length))
				{
					E_ASSERT(0);
					return false;
				}

#endif
				break;
			}

			if(!nb_png_read_uint32(file, crc32))
			{
				E_ASSERT(0);
				return false;
			}
			// TODO: check crc
		}while(!end);
		file = 0;
		// proccess data
		if(png.data == 0)
		{
			E_ASSERT(0);
			return false;
		}

		if(png.color_type == PNG_COLOR_TYPE_INDEX && png.palete == 0)
		{
			E_ASSERT(0);
			return false;
		}

		// unzip data
		{
			void * unzip_buf;
			size_t unzip_len;
			try
			{
				nb_block_unzip(unzip_buf, unzip_len, png.data, png.data_len);
			}
			catch (const char * _err)
			{
				E_ASSERT1(0, _err);
				return false;
			}
			png.data_len = unzip_len;
			png.data = (uint8*)realloc(png.data, unzip_len);
			memcpy(png.data, unzip_buf, png.data_len);
			nb_zip_free(unzip_buf);
		}

		switch(png.bit_depth)
		{
		case 1:
		case 2:
		case 4:
			if(!png.decode_1_2_4())
			{
				E_ASSERT(0);
				return false;
			}
			break;
		case 8:
			if(!png.decode_8())
			{
				E_ASSERT(0);
				return false;
			}
			break;
		default:
			E_ASSERT(0);
			return false;
		}

		_pic.w = png.result.w;
		_pic.h = png.result.h;
		_pic.data = png.result.data;
		png.result.data = 0;

		if(png.has_transparent_color)
		{
			_pic.TransparentByColor(*((uint32*)png.transparent_color));
		}
		return true;
	}

	static uint32 filter_0(uint8 * out, uint8 * pre, uint8 * cur, int w, int fu)
	{
		int sum = 0;
		int n = w * fu;
		for(int i=0; i<n; i++)
		{
			out[i] = cur[i];
			sum+= (char)out[i];
		}
		return abs(sum);
	}

	static uint32 filter_1(uint8 * out, uint8 * pre, uint8 * cur, int w, int fu)
	{
		int n = w * fu;
		int sum = 0;
		for(int i=0; i<fu; i++)
		{
			out[i] = cur[i];
			sum+= (char)out[i];
		}
		uint8 * pxs = cur + fu;
		uint8 * pxd = out + fu;
		uint8 * pa = cur;
		//uint8 * pb = pre + fu;
		//uint8 * pc = pre;
		for(int i=0; i<n-fu; i++)
		{
			uint8 v = *pxs++ - *pa++;
			*pxd++ = v;
			sum+= (char)v;
		}
		return abs(sum);
	}

	static uint32 filter_2(uint8 * out, uint8 * pre, uint8 * cur, int w, int fu)
	{
		int n = w * fu;
		int sum = 0;
		//for(int i=0; i<fu; i++)
		//{
		//	out[i] = cur[i];
		//	sum+= cur[i];
		//}
		uint8 * pxs = cur;
		uint8 * pxd = out;
		uint8 * pb = pre;
		for(int i=0; i<n; i++)
		{
			uint8 v = *pxs++ - *pb++;
			*pxd++ = v;
			sum+= (char)v;
		}
		return abs(sum);
	}

	static uint32 filter_3(uint8 * out, uint8 * pre, uint8 * cur, int w, int fu)
	{
		int n = w * fu;
		int sum = 0;
		for(int i=0; i<fu; i++)
		{
			out[i] = cur[i] - pre[i] / 2;
			sum+= (char)out[i];
		}
		uint8 * pxs = cur + fu;
		uint8 * pxd = out + fu;
		uint8 * pa = cur;
		uint8 * pb = pre + fu;
		uint8 * pc = pre;
		for(int i=0; i<n-fu; i++)
		{
			uint8 v = *pxs++ - uint8((int(*pa++) + int(*pb++)) / 2);
			*pxd++ = v;
			sum+= (char)v;
		}
		return abs(sum);
	}


	static uint32 filter_4(uint8 * out, uint8 * pre, uint8 * cur, int w, int fu)
	{
		int n = w * fu;
		int sum = 0;
		for(int i=0; i<fu; i++)
		{
			int aaa = 0;
			int bbb = pre[i];
			int ccc = 0;
			int p = aaa + bbb - ccc;
			int p_a = p - aaa;
			int p_b = p - bbb;
			int p_c = p - ccc;
			int pa = p_a < 0 ? -p_a : p_a;
			int pb = p_b < 0 ? -p_b : p_b;
			int pc = p_c < 0 ? -p_c : p_c;
			int pr = (pa <= pb && pa <= pc) ? aaa : ((pb <= pc ) ? bbb : ccc);

			out[i] = cur[i] - pr;
			sum+= (char)out[i];
		}
		uint8 * pxs = cur + fu;
		uint8 * pxd = out + fu;
		uint8 * pa = cur;
		uint8 * pb = pre + fu;
		uint8 * pc = pre;
		for(int i=0; i<n-fu; i++)
		{
			int aaa = *pa++;
			int bbb = *pb++;
			int ccc = *pc++;
			int p = aaa + bbb - ccc;
			int p_a = p - aaa;
			int p_b = p - bbb;
			int p_c = p - ccc;
			int pa = p_a < 0 ? -p_a : p_a;
			int pb = p_b < 0 ? -p_b : p_b;
			int pc = p_c < 0 ? -p_c : p_c;
			int pr = (pa <= pb && pa <= pc) ? aaa : ((pb <= pc ) ? bbb : ccc);
			uint8 v = (*pxs++) - pr;
			*pxd++ = v;
			sum+= (char)v;
		}
		return abs(sum);
	}

	static void filter_for_save(uint8 * buf, int w, int h, int fu)
	{
		// c b
		// a x
		int n = w * fu;
		uint8* tmp[5];
		tmp[0] = (uint8*)malloc(n);
		tmp[1] = (uint8*)malloc(n);
		tmp[2] = (uint8*)malloc(n);
		tmp[3] = (uint8*)malloc(n);
		tmp[4] = (uint8*)malloc(n);

		int sum[5] = {0};
		int v;
		uint32 value = 0;
		uint32 select = 0;
		uint8 * pre0 = (uint8*)malloc(n);
		memset(pre0, 0, n);
		uint8 * pre = pre0;
		uint8 * cur = buf;
		for(int y=0; y<h; y++)
		{
			v = filter_0(tmp[0], pre, cur+1, w, fu);
			value = v;
			v = filter_1(tmp[1], pre, cur+1, w, fu);
			if(v < value)
			{
				value = v;
				select = 1;
			}
			v = filter_2(tmp[2], pre, cur+1, w, fu);
			if(v < value)
			{
				value = v;
				select = 2;
			}
			v = filter_3(tmp[3], pre, cur+1, w, fu);
			if(v < value)
			{
				value = v;
				select = 3;
			}
			v = filter_4(tmp[4], pre, cur+1, w, fu);
			if(v < value)
			{
				value = v;
				select = 4;
			}
			sum[select]++;
			cur[0] = select;
			memcpy(pre, cur+1, n);
			memcpy(cur+1, tmp[select], n);
			cur+= n+1;
		}

		free(pre0);
		free(tmp[0]);
		free(tmp[1]);
		free(tmp[2]);
		free(tmp[3]);
		free(tmp[4]);
	}

	static bool SavePng(Image & _pic, const Path & _path, bool _keep_alpha)
	{
		E_TRACE_LINE(L"[nb] Save png: " + _path.GetString());

		if(_pic.data == 0 || _pic.w == 0 || _pic.h == 0)
		{
			E_ASSERT(0);
			return false;
		}
		uint32 pixel_size = _keep_alpha ? 4 : 3;
		uint32 filterd_len = _pic.w * _pic.h * pixel_size + _pic.h;
		uint8* filterd_buf = (uint8*)malloc(filterd_len);
		
		// copy image data
		{
			uint32 src_line_size = _pic.w * 4;
			uint8* p0 = _pic.data;
			uint8* p = filterd_buf;
			if(_keep_alpha)
			{
				for(int y=0; y<_pic.h; y++)
				{
					*p++ = 0;
					memcpy(p, p0, src_line_size);
					p0+=src_line_size;
					p+=src_line_size;
				}
			}
			else
			{
				for(int y=0; y<_pic.h; y++)
				{
					*p++ = 0;
					for(int x=0; x<_pic.w; x++)
					{
						*p++ = *p0++;
						*p++ = *p0++;
						*p++ = *p0++;
						p0++;
					}
				}
			}
		}

		// filtering
		if(_pic.w * _pic.h > 100)
		{
			filter_for_save(filterd_buf, _pic.w, _pic.h, pixel_size);
		}

		// compress
		void * zip_buf;
		size_t zip_len;
		try
		{
			nb_block_zip(zip_buf, zip_len, filterd_buf, filterd_len, 1);
		}
		catch (const char *)
		{
			E_ASSERT(0);
			return false;
		}
		free(filterd_buf);
		filterd_buf = 0;

		FileRef file = FS::OpenFile(_path, true);
		if(!file 
			|| !file->SetSize(0)
			|| !file->Write(nb_png_sig, 8))
		{
			E_ASSERT(0);
			nb_zip_free(zip_buf);
			return false;
		}

		uint8 header[21] = {0};
		//{0,0,0,0, 0,0,0,0, 8, 6, 0, 0, 0 };
		header[0] = 0;
		header[1] = 0;
		header[2] = 0;
		header[3] = 13;
		header[4] = 'I';
		header[5] = 'H';
		header[6] = 'D';
		header[7] = 'R';

		header[8]  = (_pic.w >> 3*8) & 0xff;
		header[9]  = (_pic.w >> 2*8) & 0xff;
		header[10] = (_pic.w >> 1*8) & 0xff;
		header[11] = (_pic.w >> 0*8) & 0xff;
		header[12] = (_pic.h >> 3*8) & 0xff;
		header[13] = (_pic.h >> 2*8) & 0xff;
		header[14] = (_pic.h >> 1*8) & 0xff;
		header[15] = (_pic.h >> 0*8) & 0xff;
		header[16] = 8; // 8bit
		header[17] = _keep_alpha ? 6 : 2; // rgba or rgb

		//uint32 header_crc32 = nb_crc32_block(header+4, sizeof(header)-4);
		uint32 header_crc32 = 0xffffffff;
		for(int i=4; i<sizeof(header); i++)
		{
			nb_crc32(header_crc32, header[i]);
		}
		header_crc32 = header_crc32 ^ 0xffffffff;


		if( !file->Write(header, sizeof(header))
			|| !nb_png_write_uint32(file, header_crc32)
			)
		{
			E_ASSERT(0);
			nb_zip_free(zip_buf);
			return false;
		}

#ifdef NB_PNG_SAVE_SOFTWARE_NAME
		{
			stringa name = Env::GetShortName();
			if(name.empty())
			{
				name = "nbug";
			}
			stringa keyword = "Software";
			uint32 chunk_size = name.length() + keyword.length() + 1;
			uint8 * pk = (uint8*) &keyword[0];
			uint8 * pn = (uint8*) &name[0];

			uint8 chunk_size_type[] =
			{
				(chunk_size >> 3*8) & 0xff,
				(chunk_size >> 2*8) & 0xff,
				(chunk_size >> 1*8) & 0xff,
				(chunk_size >> 0*8) & 0xff,
				't', 'E', 'X', 't'
			};

			uint32 data_crc32 = 0xffffffff;
			for(int i=4; i<8; i++)
			{
				nb_crc32(data_crc32, chunk_size_type[i]);
			}
			for(int i=0; i<keyword.length(); i++)
			{
				nb_crc32(data_crc32, pk[i]);
			}
			nb_crc32(data_crc32, 0);
			for(int i=0; i<name.length(); i++)
			{
				nb_crc32(data_crc32, pn[i]);
			}
			data_crc32 = data_crc32 ^ 0xffffffff;

			if( !file->Write(chunk_size_type, 8)
				|| !file->Write(pk, keyword.length())
				|| !file->Write("", 1)
				|| !file->Write(pn, name.length())
				|| !nb_png_write_uint32(file, data_crc32)
				)
			{
				E_ASSERT(0);
				nb_zip_free(zip_buf);
				return false;
			}
		}

#endif
		uint8 * src = (uint8 *)zip_buf;
		uint32 len = zip_len;
		const int proper_data_chunk_size = 1024 * 512;
		while(len)
		{
			uint32 chunk_size = len < proper_data_chunk_size ? len : proper_data_chunk_size;

			uint8 chunk_size_type[] =
			{
				(chunk_size >> 3*8) & 0xff,
				(chunk_size >> 2*8) & 0xff,
				(chunk_size >> 1*8) & 0xff,
				(chunk_size >> 0*8) & 0xff,
				'I', 'D', 'A', 'T'
			};

			uint32 data_crc32 = 0xffffffff;
			for(int i=4; i<8; i++)
			{
				nb_crc32(data_crc32, chunk_size_type[i]);
			}
			for(int i=0; i<chunk_size; i++)
			{
				nb_crc32(data_crc32, src[i]);
			}
			data_crc32 = data_crc32 ^ 0xffffffff;

			if( !file->Write(chunk_size_type, 8)
				|| !file->Write(src, chunk_size)
				|| !nb_png_write_uint32(file, data_crc32)
				)
			{
				E_ASSERT(0);
				nb_zip_free(zip_buf);
				return false;
			}
			src+= chunk_size;
			len-= chunk_size;
		}

		nb_zip_free(zip_buf);

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
			return false;
		}

		return true;
	}

	Image::Image()
	{
		// NB_PROFILE_INCLUDE;
		w = 0;
		h = 0;
		data = 0;
	}

	Image::Image(const Image & _r)
	{
		w = 0;
		h = 0;
		data = 0;
		if(_r.data)
		{
			Alloc(_r.w, _r.h);
			memcpy(data, _r.data, w * h * 4);
		}
	}

	Image & Image::operator=(const Image & _r)
	{
		if(this != &_r)
		{
			if(_r.data)
			{
				Alloc(_r.w, _r.h);
				memcpy(data, _r.data, w * h * 4);
			}
			else
			{
				Delete();
			}
		}
		return *this;
	}

	Image::~Image()
	{
		// NB_PROFILE_INCLUDE;
		free(data);
	}

	bool Image::Load(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		free(data);
		data = 0;
		return LoadPng(*this, _path);
	}

	bool Image::Save(const Path & _path, bool _keep_alpha)
	{
		// NB_PROFILE_INCLUDE;
		if(data == 0)
		{
			return false;
		}
		return SavePng(*this, _path, _keep_alpha);
	}

	void Image::Delete()
	{
		free(data);
		data = 0;
	}


	bool Image::Alloc(uint _w, uint _h)
	{
		// NB_PROFILE_INCLUDE;
		if(data != 0 && w == _w && h == _h)
		{
			return true;
		}
		Delete();
		if(_w == 0 || _h == 0)
		{
			return false;
		}

		w = _w;
		h = _h;
		data = (uint8*) malloc(w * h * 4);
		return true;
	}


	bool Image::SwapChannel(uint _a, uint _b)
	{
		//// NB_PROFILE_INCLUDE;
		E_BASIC_ASSERT(_a < 4 && _b < 4);
		if(data == 0)
		{
			return false;
		}
		uint8 * p = data;
		uint8 * end = p + w * h * 4;
		while(p < end)
		{
			uint8 tmp = p[_a];
			p[_a] =  p[_b];
			p[_b] = tmp;
			p+= 4;
		}
		return true;
	}
	bool Image::GetSubImage(Image & _sub, uint _x, uint _y, uint _w, uint _h)
	{
		//// NB_PROFILE_INCLUDE;
		if(data==0)
		{
			return false;
		}

		if(!_sub.Alloc(_w, _h))
		{
			return false;
		}

		for(int i=_x; i<int(_w+_x); i++)
		{
			for(int j=_y; j<int(_h+_y); j++)
			{
				int i1 = i - _x;
				int j1 = j - _y;
				uint8 * p1 = _sub.data + (j1 * _w + i1) * 4;
				if(i>=0 && i<(int)w && j>=0 && j<(int)h)
				{
					uint8 * p = data + (j * w + i) * 4;
					p1[0] = p[0];
					p1[1] = p[1];
					p1[2] = p[2];
					p1[3] = p[3];
				}
				else
				{
					p1[0] = 0;
					p1[1] = 0;
					p1[2] = 0;
					p1[3] = 0;
				}
			}
		}

		return _sub.data != 0;
	}

	bool Image::CopyRect(uint _x, uint _y, Image & _src, uint _x0, uint _y0, uint _w0, uint _h0)
	{
		//// NB_PROFILE_INCLUDE;
		if(_src.data ==0 || this->data == 0)
		{
			E_BASIC_ASSERT(0);
			return false;
		}
		if(_x0 + _w0 > _src.w || _y0 + _h0 > _src.h)
		{
			E_BASIC_ASSERT(0);
			return false;
		}
		if(_x >= w || _y >= h)
		{
			E_BASIC_ASSERT(0);
			return false;
		}
		uint rd = _x + _w0;
		if(rd >= w)
		{
			rd = w - 1;
		}
		uint bd = _y + _h0;
		if(bd >= h)
		{
			bd = h - 1;
		}

		for(uint j = 0; j < _h0; j++)
		{
			int ys = _y0 + j;
			int yd = _y + j;
			for(uint i = 0; i < _w0; i++)
			{
				int xs = _x0 + i;
				int xd = _x + i;
				uint8 * s = _src.Get(xs, ys);
				uint8 * d = Get(xd, yd);
				*((uint32*)d) = *((uint32*)s);
			}
		}

		return true;
	}

	void Image::TransparentByColor(uint32 _v)
	{
		if(data)
		{
			int n = w * h;
			uint32 * p1 = (uint32 *) data;
			uint32 * p2 = p1 + n;
			for(; p1 < p2; p1++)
			{
				if(*p1 == _v)
				{
					*p1 = 0;
				}
			}
		}
	}

	void Image::Fill(uint32 _v)
	{
		if(data)
		{
			int n = w * h;
			uint32 * p1 = (uint32 *) data;
			uint32 * p2 = p1 + n;
			for(; p1 < p2; p1++)
			{
				*p1 = _v;
			}
		}
	}

	void Image::FillChannel(int _channel, uint8 _v)
	{
		if(data)
		{
			int n = w * h * 4;
			uint8 * p1 = data + _channel;
			uint8 * p2 = p1 + n;
			for(; p1 < p2; p1+= 4)
			{
				*p1= _v;
			}
		}

	}
/*
	static inline void FastBlur_Point(Image & _out, Image & _in, int _x, int _y)
	{
		uint8 * sum = _out.Get(_x, _y);
		*((uint32 *) sum) = 0;
		for(int i = -1; i <= 1; i++)
		{
			for(int j = -1; j <= 1; j ++)
			{
				if(i==0 && j == 0)
				{
					continue;
				}
				int xx = _x + i;
				int yy = _y + j;
				if(xx < 0 )
				{
					xx = 0;
				}
				if(xx >= (int)_in.w)
				{
					xx = (int)_in.w - 1;
				}
				if(yy < 0)
				{
					yy = 0;
				}
				if(yy >= (int)_in.h)
				{
					yy = (int)_in.h - 1;
				}
				//float f = 1.0f / 9.0f;
				uint8 * p = _in.Get(xx, yy);
				sum[0]+= p[0];
				sum[1]+= p[1];
				sum[2]+= p[2];
				sum[3]+= p[3];
			}
		}
	}
	*/
/*
	void Image::ShiftRightAllBytes(uint _n)
	{
		if(_n >= 8)
		{
			Fill(0);
			return ;
		}

		int sz = w * h * 4;
		uint8 * p = data;

		ProcessorInfo pi;
		Env::GetProcessorInfo(pi);
		if(pi.supportSSE2)
		{
			uint8 mask[16];
			memset(mask, 0xff >> _n, sizeof(mask));
			__m128i m0 =  _mm_loadu_si128((__m128i*)mask);
			__m128i m3 =  {_n};
			int c0 = sz / 16;
			while(c0--)
			{
				__m128i m1 = _mm_loadu_si128((__m128i*)p);
				m1 = _mm_srl_epi32 (m1, m3);
				_mm_storeu_si128 ((__m128i*)p, _mm_and_si128(m1, m0));
				p+=16;
			}
			uint8 * p2 = data + sz;
			while(p < p2)
			{
				(*p++)>>= _n;
			}
		}
		else
		{
			uint8 * p2 = data + sz;
			while(p < p2)
			{
				(*p++)>>= _n;
			}
		}
	}

	bool FastBlur(Image & _out, Image & _in)
	{
		_out.Alloc(_in.w, _in.h);
		if(_in.data == 0 || _in.w < 3 || _in.h < 3)
		{
			return false;
		}

		_in.ShiftRightAllBytes(3); // all pixel reduce to 1/8

		int x1 = (int)_in.w - 2;
		int y1 = (int)_in.h - 1;

		for(int y2 = 0; y2 <= y1; y2++)
		{
			FastBlur_Point(_out, _in, 0, y2);
			FastBlur_Point(_out, _in, _in.w-1, y2);
		}
		for(int x2 = 1; x2 <= x1; x2++)
		{
			FastBlur_Point(_out, _in, x2, 0);
			FastBlur_Point(_out, _in, x2, y1);
		}

		ProcessorInfo pi;
		Env::GetProcessorInfo(pi);
		if(pi.supportSSE2)
		{
			for(int y = 1; y < y1; y++)
			{
				int wrap = (_in.w - 2) * 4;
				uint8 * sum = _out.Get(1, y);
				uint8 * src = _in.Get(0, y-1);
				int c0 = x1 >> 2;
				int c1 = x1 & 0x3;
				while(c0--)
				{
					char * p = (char*)src;
					__m128i m0 = _mm_setzero_si128();
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= 4;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= 4;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= wrap;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= 8;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= wrap;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= 4;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					p+= 4;
					m0 = _mm_adds_epu8(m0, _mm_loadu_si128((__m128i*)p));
					_mm_storeu_si128 ((__m128i*)sum, m0);
					sum+=16;
					src+=16;
				}

				switch(c1)
				{
				case 1:
					{
						char * p = (char*)src;
						__m128i m0 = _mm_setzero_si128();
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= 8;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p[3], p[2], p[1], p[0]));
						uint8 buf[16];
						_mm_storeu_si128 ((__m128i*)buf, m0);
						memcpy(sum, buf, 4);
					}
					break;
				case 2:
					{
						char * p = (char*)src;
						__m128i m0 = _mm_setzero_si128();
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 8;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						uint8 buf[16];
						_mm_storeu_si128 ((__m128i*)buf, m0);
						memcpy(sum, buf,8);
					}
					break;
				case 3:
					{
						char * p = (char*)src;
						__m128i m0 = _mm_setzero_si128();
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 8;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= wrap;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						p+= 4;
						m0 = _mm_adds_epu8(m0, _mm_set_epi8(0, 0, 0, 0, p[11], p[10], p[9], p[8], p[7], p[6], p[5], p[4], p[3], p[2], p[1], p[0]));
						uint8 buf[16];
						_mm_storeu_si128 ((__m128i*)buf, m0);
						memcpy(sum, buf, 12);
					}
					break;
				}
			}
		}
		else
		{
			for(int y = 1; y < y1; y++)
			{
				uint8 * sum = _out.Get(1, y);
				for(int x = 1; x <= x1; x++)
				{
					*((uint32 *) sum) = 0;
					for(int i = -1; i <= 1; i++)
					{
						for(int j = -1; j <= 1; j ++)
						{
							if(i==0 && j == 0)
							{
								continue;
							}
							int xx = x + i;
							int yy = y + j;
							uint8 * p = _in.Get(xx, yy);
							sum[0]+= p[0];
							sum[1]+= p[1];
							sum[2]+= p[2];
							sum[3]+= p[3];
						}
					}
					sum+=4;
				}
			}
		}
		return true;
	}

	bool Image::FastBlur()
	{
		Image pic1;
		if(e::FastBlur(pic1, *this))
		{
			Delete();
			data = pic1.data;
			pic1.data = 0;
			w = pic1.w;
			h = pic1.h;
			return true;
		}
		else
		{
			return false;
		}
	}
	*/

	static void _BlurHorizontal(Image & _out, Image & _in, uint32 _radius)
	{
		int n = _radius;
		int r,g,b,a, div;

		uint32 w = _in.w;
		uint32 h = _in.h;

		uint8 * s = _in.data;
		uint8 * d = _out.data;
		int row_pitch = 4 * w;
		for(int y=0; y<h; y++)
		{
			uint8 * p = d;
			uint8 * p0 = s;
			a = g = b = r = div = 0;
			for(int i=0; i<n; i++)
			{
				div++;
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
			}
			div = n;
			for(int x=0; x<=n; x++)
			{
				div++;
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
			}
		//	E_ASSERT(div == n + 1 + n);
			uint8 * p1 = s;
			for(int x=n+1; x<w-n; x++)
			{
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
				r-= *p1++;
				g-= *p1++;
				b-= *p1++;
				a-= *p1++;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
			}


			for(int x=w-n; x<w; x++)
			{
				div--;
				r-= *p1++;
				g-= *p1++;
				b-= *p1++;
				a-= *p1++;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
			}

			s+= row_pitch;
			d+= row_pitch;
		}
	}

	void Image::BlurHorizontal(uint32 _radius)
	{
		if(w == 0 || h == 0 || data == 0 )
		{
			return;
		}

		if(_radius > w/2)
		{
			_radius = w/2;
		}

		if(_radius == 0)
		{
			return ;
		}

		Image pic1;
		pic1.Alloc(w, h);
		_BlurHorizontal(pic1, *this, _radius);
		Delete();
		data = pic1.data;
		pic1.data = 0;
	}

	static void _BlurVertical(Image & _out, Image & _in, uint32 _radius)
	{
		int n = _radius;
		int r,g,b,a, div;

		uint32 w = _in.w;
		uint32 h = _in.h;

		uint8 * s = _in.data;
		uint8 * d = _out.data;
		int row_pitch = 4 * w;
		int row_pitch1 = row_pitch - 4;
		for(int x=0; x<w; x++)
		{
			uint8 * p = d;
			uint8 * p0 = s;
			a = g = b = r = div = 0;
			for(int i=0; i<n; i++)
			{
				div++;
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
				p0+= row_pitch1;
			}
			div = n;
			for(int y=0; y<=n; y++)
			{
				div++;
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
				p0+= row_pitch1;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
				p+= row_pitch1;
			}
		//	E_ASSERT(div == n + 1 + n);
			uint8 * p1 = s;
			for(int y=n+1; y<h-n; y++)
			{
				r+= *p0++;
				g+= *p0++;
				b+= *p0++;
				a+= *p0++;
				p0+= row_pitch1;
				r-= *p1++;
				g-= *p1++;
				b-= *p1++;
				a-= *p1++;
				p1+= row_pitch1;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
				p+= row_pitch1;
			}


			for(int y=h-n; y<h; y++)
			{
				div--;
				r-= *p1++;
				g-= *p1++;
				b-= *p1++;
				a-= *p1++;
				p1+= row_pitch1;
				*p++= r / div;
				*p++= g / div;
				*p++= b / div;
				*p++= a / div;
				p+= row_pitch1;
			}

			s+=4;
			d+=4;
		}
	}

	void Image::BlurVertical(uint32 _radius)
	{
		if(w == 0 || h == 0 || data == 0 )
		{
			return;
		}

		if(_radius > h/2)
		{
			_radius = h/2;
		}

		if(_radius == 0)
		{
			return ;
		}
		Image pic1;
		pic1.Alloc(w, h);
		_BlurVertical(pic1, *this, _radius);
		Delete();
		data = pic1.data;
		pic1.data = 0;
	}

	void Image::Blur(uint32 _radius)
	{
		BlurHorizontal(_radius);
		BlurVertical(_radius);
	}

	void Image::FlipV()
	{
		if(data)
		{
			int row_pitch = 4 * w;
			uint8 * tmp = (uint8*) malloc(row_pitch);
			for(int y=0; y<this->h/2; y++)
			{
				uint8 * p1 = data + row_pitch * y;
				uint8 * p2 = data + row_pitch * (h - y-1);
				memcpy(tmp, p1, row_pitch);
				memcpy(p1, p2, row_pitch);
				memcpy(p2, tmp, row_pitch);

			}
			free(tmp);
		}
	}
	void Image::FlipH()
	{
		if(data)
		{
			int row_pitch = 4 * w;
			for(int y=0; y<this->h; y++)
			{
				uint8 * p1 = data + row_pitch * y;
				uint8 * p2 = p1 + row_pitch -1;
				while(p1 < p2)
				{
					uint8 tmp = *p1;
					*p1++ = *p2;
					*p2-- = tmp;
				}
			}
		}
	}

	void Image::CorrectNonzeroTransparent()
	{
		if(data)
		{
			uint8 * end = data + 4 * w * h;
			for(uint8 * p = data; p < end; p+=4)
			{
				if(p[3] == 0)
				{
					p[0] = p[1] = p[2] = 0;
				}
			}
		}
	}

	bool Image::HasNonzeroTransparent()
	{
		bool ret = false;
		if(data)
		{
			uint8 * end = data + 4 * w * h;
			for(uint8 * p = data; p < end; p+=4)
			{
				if(p[3] == 0 && (p[0] || p[1] || p[2]))
				{
					ret = true;
					break;
				}
			}
		}
		return ret;
	}



}
