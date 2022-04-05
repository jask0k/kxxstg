#include <string.h>
#include <nbug/core/def.h>
#include <nbug/core/debug.h>
#include <nbug/core/zip.h>
#include <nbug/core/huffman.h>
#include <nbug/tl/map.h>
#include <nbug/tl/array.h>

namespace e
{
//#	define NB_ZIP_DUMP_HUFFMAN_CODES
//#	define NB_ZIP_DUMP_LZ77
#	define NB_UNZIP_VERIFY_CHECKSUM

	static const int NB_ZIP_TRY_MATCH = 2; // 2 or 3
	static const int NB_ZIP_MIN_WORD_LEN = 3; // must be 3
	static const int NB_ZIP_MAX_WORD_LEN = 258; // 3 ~ 258
	static const int NB_ZIP_BLOCK_SIZE = 16728;

	static const int NB_ZIP_HASH_KEY_MASK  = 0x00003FFF; 
	static const int NB_ZIP_HASH_TABE_SIZE = 4096*4;

	static const int NB_ZIP_HASH_ROOM_COUNT = 8; // 8
	static const int NB_ZIP_HASH_ROOM_COUNT_MASK = 0x07;
	static const int hash_node_lookup_order[NB_ZIP_HASH_ROOM_COUNT][NB_ZIP_HASH_ROOM_COUNT] =
	{
		{7, 6, 5, 4, 3, 2, 1, 0},
		{0, 7, 6, 5, 4, 3, 2, 1},
		{1, 0, 7, 6, 5, 4, 3, 2},
		{2, 1, 0, 7, 6, 5, 4, 3},
		{3, 2, 1, 0, 7, 6, 5, 4},
		{4, 3, 2, 1, 0, 7, 6, 5},
		{5, 4, 3, 2, 1, 0, 7, 6},
		{6, 5, 4, 3, 2, 1, 0, 7},
	};


	void nb_crc32(uint32 & _crc, uint8 _byte)
	{
		static const uint32 _crc32_table[256] =
		{
			0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
			0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
			0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
			0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
			0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
			0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
			0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
			0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
			0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
			0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
			0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
			0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
			0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
			0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
			0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
			0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
		};
		_crc = _crc32_table[(_crc ^ _byte) & 0xff] ^ (_crc >> 8);
	}

	static const uint32 low_bits_mask_32[33] =
	{
		0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff, 0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff,
	};

	uint8 nb_reverse_byte[256] =
	{
		0x0, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x8, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x4, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0xC, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x2, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0xA, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x6, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0xE, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x1, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x9, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x5, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0xD, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x3, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0xB, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x7, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0xF, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
	};

	uint32 nb_reverse_bits(uint32 _src, uint32 _bit_count)
	{
		_src<<= 32 - _bit_count;
		uint32 ret=0;
		uint8 * s = (uint8*)&_src;
		uint8 * d = (uint8*)&ret;
		d[0] = nb_reverse_byte[s[3]];
		d[1] = nb_reverse_byte[s[2]];
		d[2] = nb_reverse_byte[s[1]];
		d[3] = nb_reverse_byte[s[0]];
		return ret;
	}

	/*
	Code Bits length()(s) Code Bits Lengths   Code Bits length()(s)
	---- ---- ------     ---- ---- -------   ---- ---- -------
		257   0     3       267   1   15,16     277   4   67-82
		258   0     4       268   1   17,18     278   4   83-98
		259   0     5       269   2   19-22     279   4   99-114
		260   0     6       270   2   23-26     280   4  115-130
		261   0     7       271   2   27-30     281   5  131-162
		262   0     8       272   2   31-34     282   5  163-194
		263   0     9       273   3   35-42     283   5  195-226
		264   0    10       274   3   43-50     284   5  227-257
		265   1  11,12      275   3   51-58     285   0    258
		266   1  13,14      276   3   59-66
		*/

	const static uint32 _length_extra_table[] =
	{
		0, 0, 0, 0, 0,  0, 0, 0, 1, 1, 
		1, 1, 2, 2, 2,  2, 3, 3, 3, 3, 
		4, 4, 4, 4, 5,  5, 5, 5, 0,
	};

	const static uint32 _length_offset_table[] =
	{
		3,  4,  5,  6,   7,      8,   9,   10,  11,  13,
		15, 17, 19, 23,  27,     31,  35,  43,  51,  59,
		67, 83, 99, 115, 131,    163, 195, 227, 258, 999999999
	};

	/*
			Extra           Extra               Extra
		Code Bits Dist  Code Bits   Dist     Code Bits Distance
		---- ---- ----  ---- ----  ------    ---- ---- --------
		0   0    1     10   4     33-48    20    9   1025-1536
		1   0    2     11   4     49-64    21    9   1537-2048
		2   0    3     12   5     65-96    22   10   2049-3072
		3   0    4     13   5     97-128   23   10   3073-4096
		4   1   5,6    14   6    129-192   24   11   4097-6144
		5   1   7,8    15   6    193-256   25   11   6145-8192
		6   2   9-12   16   7    257-384   26   12  8193-12288
		7   2  13-16   17   7    385-512   27   12 12289-16384
		8   3  17-24   18   8    513-768   28   13 16385-24576
		9   3  25-32   19   8   769-1024   29   13 24577-32768
	*/
	const static uint32 _distance_extra_table[] =
	{
		0, 0, 0, 0,
		1, 1,
		2, 2,
		3, 3,
		4, 4,
		5, 5,
		6, 6,
		7, 7,
		8, 8,
		9, 9,
		10, 10,
		11, 11,
		12, 12,
		13, 13,
	};

	const static uint32 _distance_offset_table[] =
	{
		1, 2, 3, 4,
		5, 7,
		9, 13,
		17, 25,
		33, 49,
		65, 97,
		129, 193,
		257, 385,
		513, 769,
		1025, 1537,
		2049, 3073,
		4097, 6145,
		8193, 12289,
		16385, 24577,
		999999999
	};

	// code lengths for the code length alphabet, from the spec
	const static uint32 _clcl_table[19] = 
	{16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

#ifdef NB_DEBUG
	template<typename T> string bits_to_str(T _n, int _sz)
	{
		string s;
		for(int i=0; i<_sz; i++)
		{
			if(((_n >> i) & 0x01) == 0)
			{
				s+= "0";
			}
			else
			{
				s+= "1";
			}
		}

		return s;
	}

	void DecodeHuffman32::DumpTable(TB * table, uint8 len, int _indent)
	{
		if(table == 0)
		{
			table = root;
			len = root_len;
		}
		if(table == 0)
		{
			return;
		}
		uint32 sz = 0x00000001 << len;
		for(int code=0; code<sz; code++)
		{
			TB * entry = table + code;
			for(int i=0; i<_indent; i++)
			{
				E_TRACE(L" ");
			}

			if(entry->type == TB::typeTable)
			{
				string cs = bits_to_str(code, len);
				E_TRACE_LINE(cs + L" +");
				DumpTable(entry->next, entry->len, _indent+len);
			}
			else if(entry->type == TB::typeSymbol)
			{
				string cs = bits_to_str(code, entry->len);
				E_TRACE(cs);
				for(int i=entry->len; i<len; i++)
				{
					E_TRACE(L"_");
				}
				E_TRACE_LINE(L"");
			}
			else
			{
				for(int i=0; i<len; i++)
				{
					E_TRACE(L"_");
				}
				E_TRACE_LINE(L"");
			}
		}
	}

#endif

	struct SLC
	{
		uint32 sym;
		uint32 code;
		uint8  len;
	};

	void DecodeHuffman32::free_table_block(TB * & _table, uint8 _len)
	{
		uint32 sz = 0x00000001 << _len;
		for(int i=0; i<sz; i++)
		{
			if(_table[i].type == TB::typeTable && _table[i].next)
			{
				free_table_block(_table[i].next, _table[i].len);
			}
		}
		free(_table);
		_table = 0;
	}

	DecodeHuffman32::DecodeHuffman32()
	{
		root = 0;
	}

	DecodeHuffman32::~DecodeHuffman32()
	{
		if(root)
		{
			free_table_block(root, root_len);
		}
	}

	bool DecodeHuffman32::add_to_table(TB * _table, uint8 _table_len, SLC * _slc, SLC * _end, uint8 _cut)
	{
		uint8 len = _slc->len - _cut;
		uint32 code = _slc->code >> _cut;
		int fill_len = _table_len - len;
		if(fill_len >= 0)
		{
			uint32 count = 0x01 << fill_len;
			for(uint32 i=0; i<count; i++)
			{
				uint32 c = code | (i << len);
				TB * entry = _table + c;

				E_ASSERT(entry->type != TB::typeTable && entry->len != len);
				if(entry->type == TB::typeUnused || entry->len > len)
				{
					entry->len = len;
					entry->sym =_slc->sym;
					entry->type = TB::typeSymbol;
				}
			}
		}
		else
		{
			uint32 c = low_bits_mask_32[_table_len] & code;
			TB * entry = _table + c;
			if(entry->type != TB::typeSymbol)
			{
				if(entry->type != TB::typeTable)
				{
					uint8 max_len = 0;
					for(SLC * p1 = _end-1; p1>=_slc; p1--)
					{
						if((low_bits_mask_32[_table_len] & (p1->code >> _cut)) == c)
						{
							max_len = p1->len - _cut - _table_len;
							break;
						}
					}

					E_ASSERT(len > 0);
					entry->len = max_len;
					if(entry->len > 8)
					{
						if((entry->len >> 1) > 10)
						{
							entry->len = 8;
						}
						else
						{
							entry->len >>= 1;
						}
					}

					// create sub table
					uint32 sub_size = 0x00000001 << entry->len;
					entry->next = (TB*) malloc(sizeof(TB) * sub_size);
					memset(entry->next, 0, sizeof(TB) * sub_size);
					entry->type = TB::typeTable;

				}
				return add_to_table(entry->next, entry->len, _slc, _end, _cut + _table_len);
			}
			else
			{
				E_ASSERT1(0, string(bits_to_str(c, 5)) + L" is prefix of " + string(bits_to_str(code, len)) );
				return false;
			}
		}
		return true;
	}

	static int compare_slc(const void * _l, const void * _r)
	{
		return int(((SLC*) _l)->len) - int(((SLC*) _r)->len);
	}

	bool DecodeHuffman32::Init(uint8 * _lengths, size_t _sym_count, int _codetype)
	{
		E_ASSERT(_codetype == 0 || _codetype == 1);
		if(root)
		{
			free_table_block(root, root_len);
		}

		max_len = 0;
		min_len = 32;

		uint32 count[33];
		memset(count, 0, sizeof(count));
		uint32 used_sym_count=0;
		for(int sym = 0; sym < _sym_count; sym++)
		{
			uint8 & len = _lengths[sym];
			if(len > 32)
			{
				E_ASSERT(0);
				return false;
			}
			if(len)
			{
				if(min_len > len && len > 0)
				{
					min_len = len;
				}
				if(max_len < len)
				{
					max_len = len;
				}
				count[len]++;
				used_sym_count++;
			}
		}

		if(used_sym_count == 0)
		{
			return false;
		}

		SLC * slc = (SLC *) malloc(sizeof(SLC) * used_sym_count);
		memset(slc, 0, sizeof(SLC) * used_sym_count);

		uint32 next_code[33];
		memset(next_code, 0, sizeof(next_code));

		if(_codetype==0)
		{
			for(int len = 1; len <= max_len; len++)
			{
				next_code[len] = (next_code[len-1] + count[len-1]) << 1;
			}

			SLC * p = slc;

#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
			E_TRACE_LINE(L"UNZIP:");
#endif
			for(int sym = 0;  sym < _sym_count; sym++)
			{
				uint32 len = _lengths[sym];
				E_ASSERT(len <= max_len);
				if(len)
				{
					uint32 code = next_code[len]++;
					code = nb_reverse_bits(code, len);
					p->len  = len;
					p->code = code;
					p->sym  = sym;
#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
					E_TRACE_LINE(string(sym) +  L": " + bits_to_str(p->code, p->len));
#endif
					p++;
				}
			}
#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
			E_TRACE_LINE(L"");
#endif
		}
		else
		{
			E_ASSERT(_codetype==1);
			SLC * p = slc;
			for(int sym=0; sym<_sym_count; sym++)
			{
				uint32 len = _lengths[sym];
				E_ASSERT(len <= max_len);
				if(len)
				{
					uint32 code = next_code[len];

					p->len  = len;
					p->code = code;
					p->sym  = sym;
					if(len<32 && (code>>len))
					{
						free(slc);
						return false;
					}

					for(int i=len; i>0; i--)
					{
						if(next_code[i] & 1)
						{
							if(i==1)
							{
								next_code[i]++;
							}
							else
							{
								next_code[i] = next_code[i-1]<<1;
							}
							break;
						}
						next_code[i]++;
					}

					for(int i=len+1; i<33; i++)
					{
						if((next_code[i]>>1) == code)
						{
							code = next_code[i];
							next_code[i] = next_code[i-1]<<1;
						}
						else
						{
							break;
						}
					}

					p++;
				}
			}

			if(used_sym_count != 1)
			{
				for(int len=1;len<33;len++)
				{
					if(next_code[len] & (0xffffffffUL>>(32-len)))
					{
						free(slc);
						return false;
					}
				}
			}

			p = slc;
			for(int sym=0; sym<_sym_count; sym++)
			{
				uint8 len = _lengths[sym];
				if(len)
				{
					uint32 temp=0;
					for(int i=0; i<len; i++)
					{
						temp<<=1;
						temp|=(p->code>>i) & 1;
					}

					p->code = temp;
					//E_TRACE_LINE(string(sym) +  L": " + bits_to_str(p->code, p->len));
					p++;
				}
			}
		}

		qsort(slc, used_sym_count, sizeof(SLC), &compare_slc);

		if(max_len <= 10)
		{
			root_len = max_len;
		}
		else
		{
			root_len = min_len+4;
		}

		if(root_len < 6)
		{
			root_len = 6;
		}

		if(root_len > max_len)
		{
			root_len = max_len;
		}

		uint32 root_size = 0x00000001 << root_len;
		root = (TB*) malloc(sizeof(TB) * root_size);
		memset(root, 0, sizeof(TB) * root_size);
		//fill_table(root, root_len, slc, sym_count, 0);
		SLC * end = slc + used_sym_count;
		// E_TRACE("sym_count" + string(sym_count));
		for(SLC * p = slc; p<end; p++)
		{
			E_ASSERT(p->len != 0);
			//E_TRACE_LINE(bits_to_str(p->code, p->len));
			//bits_to_str(p->code, p->len);
			if(!add_to_table(root, root_len, p, end, 0))
			{
				free(slc);
				return false;
			}
		}
		free(slc);

//#ifdef NB_DEBUG
//		E_TRACE_LINE(L"-------------");
//		DumpTable(0, 0);
//		E_TRACE_LINE(L"-------------");
//#endif
		return true;
	}

	static uint32 adler32(uint8 * _buf, int _len)
	{
		uint32 a = 1;
		uint32 b = 0;
		for(int i = 0; i < _len; i++)
		{
			a = (a + _buf[i]) % 65521;
			b = (b + a) % 65521;
		}

		return (b << 16) | a;
	}

	static DecodeHuffman32 * static_literal_tree = 0;
	static DecodeHuffman32 * static_distance_tree = 0;
	static void deinit_static_huffman_trees()
	{
		delete static_literal_tree;
		delete static_distance_tree;
	}

	static void init_static_huffman_trees()
	{
		E_ASSERT(static_literal_tree == 0);
		E_ASSERT(static_distance_tree == 0);

		atexit(&deinit_static_huffman_trees);
		uint8 lens[288];
		for(int i=0; i<=143; i++)
		{
			lens[i] = 8;
		}
		for(int i=144; i<=255; i++)
		{
			lens[i] = 9;
		}
		for(int i=256; i<=279; i++)
		{
			lens[i] = 7;
		}
		for(int i=280; i<=287; i++)
		{
			lens[i] = 8;
		}

		static_literal_tree = enew DecodeHuffman32();
		static_literal_tree->Init(lens, 288, 0);

		for(int i=0; i<=31; i++)
		{
			lens[i] = 5;
		}

		static_distance_tree = enew DecodeHuffman32();
		static_distance_tree->Init(lens, 30, 0);
	}

	class Unzip
	{
		void * rc;
		int (* rf)(void*, void*, unsigned int);
		void * wc;
		int (* wf)(void*, void*, unsigned int);

		int header;

		uint8  in_buf[256];
		uint8* in_cur_ptr;
		uint32 in_num;
		uint32 bit_buf;
		uint32 bit_num;

		static const int out_buf_size = 32768;
		uint8* out_buf[2];
		uint8* out_end[2];
		uint8* out_cur_ptr;
		uint8* out_cur_end;
		int out_cur_idx;

		int out_num;

		uint32 adler_a;
		union
		{
			uint32 crc;
			uint32 adler_b;
		};

		int block_count;

		bool decode_block(DecodeHuffman32 & _literal_tree, DecodeHuffman32 & distance_tree)
		{
			int sym_count = 0;
			uint32 sym;
			do
			{
				sym = _literal_tree.Decode(this);
				//E_TRACE_LINE(string(sym));
				if(sym < 0 || sym > 285)
				{
					throw(NB_SRC_LOC "corrupted data");
				}

				sym_count++;
				if(sym < 256)
				{
					write(sym);
				}
				else if(sym > 256)
				{

					int table_idx   = sym - 257;
					int extra_bits  = _length_extra_table[table_idx];
					int base_length = _length_offset_table[table_idx];

					uint32 length;
					if(extra_bits)
					{
						length = read_bits_and_drop(extra_bits) + base_length;
					}
					else
					{
						length = base_length;
					}

					uint32 dist_code = distance_tree.Decode(this);
					if(dist_code < 0 || dist_code > 29)
					{
						throw(NB_SRC_LOC "corrupted data");
					}

					extra_bits  = _distance_extra_table[dist_code];
					uint32 base_distance = _distance_offset_table[dist_code];
					uint32 distance = extra_bits ? read_bits_and_drop(extra_bits) +  base_distance : base_distance;

					if(distance > out_num)
					{
						throw(NB_SRC_LOC "corrupted data");
					}

					copy_word(distance, length);
				}

			}while(sym != 256);
			//E_TRACE_LINE(L"sym_count=" + string(sym_count));
			return true;
		}

		void flush_remain_output()
		{
			uint8 * p = out_buf[out_cur_idx];
			if(out_cur_ptr > p)
			{
				int n0 = out_cur_ptr - p;
				int n = wf(wc, p, n0);
				if(n != n0)
				{
					throw(NB_SRC_LOC "write failed");
				}
			}
			free(out_buf[0]);
			free(out_buf[1]);
			out_buf[0] = 0;
			out_buf[1] = 0;
			out_cur_ptr = 0;
		}

		bool skip_zlib_header()
		{
			// RFC 1950
			//this->require_in(16);
			uint32 tmp = read_bits(16);

			uint8 CM  = tmp & 0x0f;
			if(CM != 8)
			{
				return false;
			}

			uint8 CINFO  = (tmp >> 4) & 0x0f;
			if(CINFO > 7)
			{
				return false;
			}
			int window_size = 256 << CINFO;

			uint8 FCHECK = (tmp >> 8) & 0x1f;
			uint8 FDICT  = (tmp >> 13) & 0x01;
			uint8 FLEVEL = (tmp >> 14) & 0x03;

			if(FDICT)
			{
				return false;
			}

			uint8 CMF = (CINFO << 4) | CM;
			uint8 FLG = (FLEVEL << 6) | (FDICT << 5) | FCHECK;

			bool check = (((uint16)CMF)*256 + (uint16)FLG) % 31 == 0;
			if(check)
			{
				drop_bits(16);
				header = 1;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool skip_gzip_header()
		{
			// RFC 1952  GZIP
			uint8 ID1, ID2, CM, FLG, XFL, OS, MTIME[4];
			ID1      = this->read_bits_and_drop(8);
			ID2      = this->read_bits_and_drop(8);
			CM       = this->read_bits_and_drop(8);
			FLG      = this->read_bits_and_drop(8);
			MTIME[0] = this->read_bits_and_drop(8);
			MTIME[1] = this->read_bits_and_drop(8);
			MTIME[2] = this->read_bits_and_drop(8);
			MTIME[3] = this->read_bits_and_drop(8);
			XFL      = this->read_bits_and_drop(8);
			OS       = this->read_bits_and_drop(8);

			if(FLG & 0x07) // FEXTRA
			{
				//this->require_in(16);
				uint32 XLEN = this->read_bits_and_drop(8);
				XLEN|= uint32(this->read_bits_and_drop(8)) << 8;
				//this->require_in(XLEN*8);
				for(uint32 i=0; i<XLEN; i++)
				{
					this->read_bits_and_drop(8);
				}
			}

			if(FLG & 0x0f) // FNAME
			{
				uint8 ch;
				do
				{
					//this->require_in(8);
					ch = this->read_bits_and_drop(8);
				}while(ch);
			}

			if(FLG & 0x10) // FCOMMENT
			{
				uint8 ch;
				do
				{
					//this->require_in(8);
					ch = this->read_bits_and_drop(8);
				}while(ch);
			}

			//if(FLG & 0x01) // FTEXT
			//{
			//}

			if(FLG & 0x03) // FHCRC
			{
				//this->require_in(16);
				this->read_bits_and_drop(16);
			}
			header = 2;
			return true;
		}

		void skip_header() throw(const char *)
		{
			uint32 tmp = read_bits(16);
			if(tmp == 0x8b1f)
			{
				skip_gzip_header();
			}
			else
			{
				skip_zlib_header();
			}
			if(header == 2)
			{
				crc = 0xffffffff;
			}
		}

	public:
		void decode_stock_block()
		{
			// no compression
			// +---+---+---+---+================================+
			// |  LEN  | NLEN  |... LEN bytes of literal data...|
			// +---+---+---+---+================================+

			align_to_byte();

			//uint32 len = *this->in_cur_ptr++;
			//len = len | (uint32(*this->in_cur_ptr++) << 8);
			//uint32 nlen = *this->in_cur_ptr++;
			//nlen = nlen | (uint32(*this->in_cur_ptr++) << 8);
			uint32 len = read_bits_and_drop(16);
			uint32 nlen = read_bits_and_drop(16);

			if((len & 0xffff) !=( (~nlen) & 0xffff))
			{
				throw(NB_SRC_LOC "corrupted data");
			}

			while(len--)
			{
				uint8 tmp = read_bits_and_drop(8);
				write(&tmp, 1);
			}
		}

		void decode_static_block()
		{
			if(static_literal_tree == 0)
			{
				init_static_huffman_trees();
			}

			if(!decode_block(*static_literal_tree, *static_distance_tree))
			{
				throw(NB_SRC_LOC "corrupted data");
			}
		}

		void decode_dynamic_block()
		{
			uint32 HLIT = this->read_bits_and_drop(5);
			uint32 HDIST = this->read_bits_and_drop(5);
			uint32 HCLEN = this->read_bits_and_drop(4);

			// (HCLEN + 4) x 3 bits: code lengths for the code length
			uint8 lengths_of_lengths_tree[19];

			for(int i=0; i<HCLEN + 4; i++)
			{
				uint32 n = this->read_bits_and_drop(3);
				lengths_of_lengths_tree[_clcl_table[i]] = n;
			}

			for(int i=HCLEN + 4; i<19; i++)
			{
				lengths_of_lengths_tree[_clcl_table[i]] = 0;
			}

			//E_TRACE_LINE("");
			//for(int i=0; i<19; i++)
			//{
			//	E_TRACE_LINE(L"len[" + string(i) + L"] = " + string((int)lengths_of_lengths_tree[i]));
			//}

			DecodeHuffman32 lengths_tree;
			if(!lengths_tree.Init(lengths_of_lengths_tree, 19, 0))
			{
				throw(NB_SRC_LOC "corrupted data");
			}
			int lcount = HLIT + 257 + HDIST + 1;
			uint8 a[288+32];
			memset(a, 0, sizeof(a));
			for(int i=0; i<lcount; )
			{
				uint32 previous;
				int16 len = lengths_tree.Decode(this);
				if(len >= 0 && len <= 18)
				{
					uint32 times;
					uint32 end;
					switch(len)
					{
					case 16:
						times = this->read_bits_and_drop(2) + 3;
						end = i + times;
						while(i < end)
						{
							a[i++] = previous;
						}
						break;
					case 17:
						times = this->read_bits_and_drop(3) + 3;
						end = i + times;
						while(i < end)
						{
							a[i++] = 0;
						}
						previous = 0;
						break;
					case 18:
						times = this->read_bits_and_drop(7) + 11;
						end = i + times;
						while(i < end)
						{
							a[i++] = 0;
						}
						previous = 0;
						//i+= times;
						break;
					default:
						a[i++] = (uint8)len;
						previous = len;
						break;
					}
				}
				else
				{
					throw(NB_SRC_LOC "corrupted data");
				}
			}

			//E_TRACE_LINE("");
			//for(int i=0; i<lcount; i++)
			//{
			//	E_TRACE_LINE(L"a[" + string(i) + L"] = " + string(a[i]));
			//}

			DecodeHuffman32 literal_tree;
			if(!literal_tree.Init(a, HLIT + 257, 0))
			{
				throw(NB_SRC_LOC "corrupted data");
			}


			DecodeHuffman32 distance_tree;

			if(!(HDIST == 0 && a[HLIT + 257] == 0))
			{
				if(!distance_tree.Init(a + HLIT + 257, HDIST + 1, 0))
				{
					throw(NB_SRC_LOC "corrupted data");
				}
			}

			if(!decode_block(literal_tree, distance_tree))
			{
				throw(NB_SRC_LOC "corrupted data");
			}
	//#ifdef NB_DEBUG
	//					literal_tree.DumpDbgCounters();
	//					distance_tree.DumpDbgCounters();
	//#endif
		}

		void run() throw(const char *)
		{
			//E_TRACE_LINE(L"Unzip::run()");
			prepare_in();

			skip_header();

			E_ASSERT(out_buf[0] == 0);
			E_ASSERT(out_buf[1] == 0);

			out_num = 0;
			out_buf[0] = (uint8*) malloc(out_buf_size);
			out_buf[1] = (uint8*) malloc(out_buf_size);
			out_end[0] = out_buf[0] + out_buf_size;
			out_end[1] = out_buf[1] + out_buf_size;

			out_cur_idx = 0;
			out_cur_ptr = out_buf[out_cur_idx];
			out_cur_end = out_end[out_cur_idx];

			bool final_block;
			do
			{
				final_block = read_bits_and_drop(1) ? true : false;
				uint8 block_type = read_bits_and_drop(2);

				switch(block_type)
				{
				case 0x00:
					decode_stock_block();
					break;
				case 0x01:
					decode_static_block();
					break;
				case 0x02:
					decode_dynamic_block();
					break;
				case 0x03:
					throw(NB_SRC_LOC "invalid block (0x03)");
				}

				block_count++;
			}while(!final_block);

			flush_remain_output();
#ifdef NB_UNZIP_VERIFY_CHECKSUM
			verify_checksum();
#endif
		}
	
#ifdef NB_UNZIP_VERIFY_CHECKSUM
		void verify_checksum()
		{
			//output_align_to_byte();
			align_to_byte();
			bool check_failed = false;
			if(header == 1)
			{

				uint32 adler =  (adler_b << 16) | adler_a;
				uint32 adler0 = this->read_bits_and_drop(8);
				uint32 adler1 = this->read_bits_and_drop(8);
				uint32 adler2 = this->read_bits_and_drop(8);
				uint32 adler3 = this->read_bits_and_drop(8);
				check_failed = (adler != ((adler0 << 24) | (adler1 << 16) | (adler2 << 8)	| adler3) );
			}
			else
			{
				crc = crc ^ 0xffffffff;
				uint32 c = read_bits_and_drop(16);
				uint32 c1 = read_bits_and_drop(16);
				c = c | (c1 << 16);
				check_failed = c != crc;

				if(!check_failed)
				{
					uint32 sz = read_bits_and_drop(16);
					uint32 sz1 = read_bits_and_drop(16);
					sz = sz | (sz1 << 16);
					check_failed = sz != out_num;
				}

			}

			if(check_failed)
			{
				throw(NB_SRC_LOC "corrupted data (checksum mismatch)");
			}
		}
#endif

		Unzip(void *_read_context,
			int (*_read_func)(void*, void*, unsigned int),
			void *_write_context,
			int (*_write_func)(void*, void*, unsigned int)
			)
		{
			rc = _read_context;
			rf = _read_func;
			wc = _write_context;
			wf = _write_func;
			header = 0;

			in_cur_ptr = 0;
			in_num = 0;
			bit_buf = 0;
			bit_num = 0;


			out_buf[0] = 0;
			out_buf[1] = 0;
			out_cur_ptr = 0;
#ifdef NB_UNZIP_VERIFY_CHECKSUM
			adler_a = 1;
			adler_b = 0;
#endif
			block_count = 0;
		}

		~Unzip()
		{
			free(out_buf[0]);
			free(out_buf[1]);
		}

		void write(uint8 _sym)
		{
#ifdef NB_ZIP_DUMP_LZ77
			if(_sym >= 32 && _sym < 128)
			{
				E_TRACE_LINE(string((wchar_t)_sym) + L"' @" + string(out_num));
			}
			else
			{
				E_TRACE_LINE(ByteToHex(_sym) + L") @" + string(out_num));
			}
#endif
			E_ASSERT(out_cur_ptr < out_cur_end);

#ifdef NB_UNZIP_VERIFY_CHECKSUM
			switch(header)
			{
			case 1:
				adler_a = adler_a + _sym;
				if(adler_a >  65521)
				{
					adler_a-= 65521;
				}
				adler_b = adler_b + adler_a;
				if(adler_b > 65521)
				{
					adler_b-= 65521;
				}
				break;
			case 2:
				nb_crc32(crc, _sym);
				break;
			}
#endif

			*out_cur_ptr++ = _sym;
			out_num++;
			if(out_cur_ptr == out_cur_end)
			{
				// switch_out_buf();
				uint8 * p = out_buf[out_cur_idx];

				int n = wf(wc, p, out_buf_size);
				if(n != out_buf_size)
				{
					throw(NB_SRC_LOC "write failed");
				}

				out_cur_idx = 1 - out_cur_idx;
				out_cur_ptr = out_buf[out_cur_idx];
				out_cur_end = out_end[out_cur_idx];
			}
		}

		void write(uint8 * _buf, size_t _n)
		{
			uint8 * end = _buf + _n;
			while(_buf < end)
			{
				write(*_buf++);
			}
		}

		void copy_word(int _distance, int _length)
		{
			uint8 * p = out_cur_ptr - _distance;
			uint8 * pe;
			int pi;
			if(p < out_buf[out_cur_idx])
			{
				int dis1 = _distance - (out_cur_ptr - out_buf[out_cur_idx]);
				E_ASSERT(dis1 <= out_buf_size);
				pi = 1-out_cur_idx;
				pe = out_end[pi];
				p = pe - dis1;
			}
			else
			{
				pi = out_cur_idx;
				pe = out_cur_end;
			}
			while(_length--)
			{
				write(*p++);
				if(p == pe)
				{
					pi = 1 - pi;
					pe =  out_end[pi];
					p = out_buf[pi];
				}
			}
		}


		void prepare_in()
		{
			uint32 by_room = sizeof(uint32) - ((bit_num + 0x07) >> 3);
			E_ASSERT(by_room > 0);
			if(in_num < by_room)
			{
				for(int i=0; i<in_num; i++)
				{
					in_buf[i] = in_cur_ptr[i];
				}

				int n = rf(rc, in_buf + in_num, sizeof(in_buf) - in_num);
				if(n > 0)
				{
					in_cur_ptr = in_buf;
					in_num += n;
				}
			}
			uint32 by_read = by_room < in_num ? by_room : in_num;
			uint32 tmp = 0;
			memcpy(&tmp, in_cur_ptr, by_read);
			in_num-= by_read;
			in_cur_ptr+= by_read;
			bit_buf|= tmp << bit_num;
			bit_num+= (by_read << 3);
		}

		// no drop
		uint32 read_bits(uint32 _n)
		{
			E_ASSERT(_n >= 1 && _n <= 20);
			if(bit_num < _n)
			{
				prepare_in();
				if(bit_num < _n)
				{
					throw(NB_SRC_LOC "not enough input");
				}
			}

			return low_bits_mask_32[_n] & bit_buf;
		}

		void drop_bits(uint32 _n)
		{
			E_ASSERT(_n <= bit_num);
			bit_buf>>= _n;
			bit_num-= _n;
		}

		uint32 read_bits_and_drop(uint32 _n)
		{
			uint32 ret = read_bits(_n);
			drop_bits(_n);
			return ret;
		}

		//void require_in(uint32 _bit_count)
		//{ 
		//	uint32 bit_cout_left = (in_num << 3) + bit_num;
		//	if(bit_cout_left < _bit_count) 
		//	{
		//		throw(NB_SRC_LOC "not enough input"); 
		//	}
		//}

		void align_to_byte()
		{
			uint32 b = bit_num & 0x07;
			if(b)
			{
				bit_buf>>= b;
				bit_num-= b;
			}

			if(bit_num)
			{
				uint32 n = bit_num >> 3;
				while(n--)
				{
					*(--in_cur_ptr) = (bit_buf >> (n * 8)) & 0xff;
					in_num++;
				}
				bit_num = 0;
				bit_buf = 0;
			}
		}

	};

	void nb_zip_free(void * _p)
	{ free(_p); }


	void nb_unzip(void *_rc,
		int (*_rf)(void*, void*, unsigned int),
		void *_wc,
		int (*_wf)(void*, void*, unsigned int)
		)
	{
		// NB_PROFILE_INCLUDE;
		if( _rf == 0 || _wf == 0)
		{
			throw(NB_SRC_LOC "bad param");
		}

		Unzip ctx(_rc, _rf, _wc, _wf);
		ctx.run();
	}

	static uint32 _hash_noise[3][256];
	static bool _hash_noise_inited = false;
	static int _distance_index[32768+1];
	static int _length_index[258+1];
	class Zip
	{
		void * rc;
		int (* rf)(void*, void*, unsigned int);
		void * wc;
		int (* wf)(void*, void*, unsigned int);

		int header;

		static const int SRC_SIZE = 32768*2 + 16384;
		static const int SRC_CACHE_LINE = 32768*2 + NB_ZIP_MAX_WORD_LEN + NB_ZIP_TRY_MATCH;
		static const int SRC_CACHE_SHIFT = 32768;
		static const int SRC_END_INF = SRC_SIZE + 1;
		uint8 src[SRC_SIZE];
		int src_end; // real len if end of input, otherwise 
		int src_pos;
		int src_off;

		struct Word
		{
			int length_code : 16;
			int length_extra_bits : 8;
			int length_extra_data : 8;
			int distance_code : 8;
			int distance_extra_bits : 8;
			int distance_extra_data : 16;
		};

		Word lz77_buf[NB_ZIP_BLOCK_SIZE];
		int lz77_pos;
		
		uint32 out_buf[8];
		int out_buf_pos;
		uint32 bit_buf;
		int bit_buf_pos;

		int block_count;
		int input_total;
		int output_total;

		uint32 adler_a;
		union
		{
			uint32 adler_b;
			uint32 crc;
		};

		struct HUFFMAN_TABLE_ITEM
		{
			union
			{
				uint32 freq;
				uint32 code;
			};
			uint32 len;
		};

		HUFFMAN_TABLE_ITEM literal_huffman_table[288]; 
		HUFFMAN_TABLE_ITEM distance_huffman_table[32];
		uint32 literal_code_count;
		uint32 fake_distance_code_count;
		uint32 real_distance_code_count;

	public:
		Zip(void *_read_context,
			int (*_read_func)(void*, void*, unsigned int),
			void *_write_context,
			int (*_write_func)(void*, void*, unsigned int),
			int _header
			)
		{
			if(!_hash_noise_inited)
			{
				_hash_noise_inited = true;
				uint8 *p = (uint8*)&_hash_noise[0][0];
				uint8 *pend = p + sizeof(_hash_noise);
				while(p!=pend)
				{
					*p++ = rand() & 0xff;
				}

				for(int i=0; i<30; i++)
				{
					int a = _distance_offset_table[i];
					int b = i == 29 ? 32768 : _distance_offset_table[i+1] -1;
					for(int j=a; j<=b; j++)
					{
						_distance_index[j] = i;
					}
				}
				_distance_index[0] = 0;
				for(int i=0; i<28; i++)
				{
					int a = _length_offset_table[i];
					int b = _length_offset_table[i+1] - 1;
					for(int j=a; j<=b; j++)
					{
						_length_index[j] = i;
					}
				}
				_length_index[0] = 0;
				_length_index[1] = 0;
				_length_index[2] = 0;
				_length_index[258] = 28;
			}

			rc = _read_context;
			rf = _read_func;
			wc = _write_context;
			wf = _write_func;
			header = _header;

			src_end = SRC_END_INF;
			src_pos = 0;
			src_off = 0;

			lz77_pos = 0;
			memset(lz77_hash, 0, sizeof(lz77_hash));

			out_buf_pos = 0;
			bit_buf = 0;
			bit_buf_pos = 0;

			block_count = 0;
			input_total = 0;
			output_total = 0;

			adler_a = 1;
			adler_b = 0;


#ifndef NB_ZIP_STATIC_BLOCK_ONLY
			coin_node_pool = 0;
			coin_pool = 0;
#endif
		}

		~Zip()
		{
#ifndef NB_ZIP_STATIC_BLOCK_ONLY
			free_coin_pool();
			free_coin_node_pool();
#endif
		}

	private:
#ifndef NB_ZIP_STATIC_BLOCK_ONLY

		struct Coin
		{
			struct Node
			{
				int s;
				Node * next;
			};
			Node * symbols;
			//Array<int> symbols;
			void init(Zip * _alocator, int _symbol)
			{
				symbols = _alocator->alloc_coin_node();
				symbols->s = _symbol;
				symbols->next = 0;
			}
			union
			{
				float value;
				Coin * pool_next;
			};
			static int compare(const void * _l, const void * _r)
			{
				Coin* a = *((Coin**)_l);
				Coin* b = *((Coin**)_r);
				return a->value == b->value ? 0 : 
					(a->value < b->value ? -1 : 1);
			}

			void merge(Coin * _r)
			{
				Coin::Node * p = _r->symbols;
				while(p)
				{
					Coin::Node * p1 = p;
					p = p->next;
					p1->next = symbols;
					symbols = p1;
				}
				value+= _r->value;
				_r->symbols = 0;
			}
		};

		Coin::Node * coin_node_pool;
		Coin * coin_pool;

		void free_coin_pool()
		{
			Coin * p = coin_pool;
			while(p)
			{
				Coin * p1 = p;
				p= p->pool_next;
				free(p1);
			}
		}

		void free_coin(Coin * p)
		{
			if(p)
			{
				{
					Coin::Node * p1 = p->symbols;
					while(p1)
					{
						Coin::Node * p2 = p1;
						p1 = p1->next;
						free_coin_node(p2);
					}
				}
				p->pool_next = coin_pool;
				coin_pool = p;
			}
		}

		void free_coin_node_pool()
		{
			Coin::Node * p = coin_node_pool;
			while(p)
			{
				Coin::Node * p1 = p;
				p = p->next;
				free(p1);
			}
		}

		void free_coin_node(Coin::Node * p)
		{
			if(p)
			{
				p->next = coin_node_pool;
				coin_node_pool = p;
			}
		}

		Coin::Node * alloc_coin_node()
		{
			Coin::Node * ret;
			if(coin_node_pool)
			{
				ret = coin_node_pool;
				coin_node_pool = coin_node_pool->next;
			}
			else
			{
				ret = (Coin::Node*) malloc(sizeof(Coin::Node));
			}
			return ret;
		}

		Coin * alloc_coin()
		{
			Coin * ret;
			if(coin_pool)
			{
				ret = coin_pool;
				coin_pool = coin_pool->pool_next;
			}
			else
			{
				ret = (Coin*) malloc(sizeof(Coin));
			}
			return ret;
		}

		struct CoinRow
		{
			Array<Coin*> v;
			Zip * alocator;
			void clear()
			{
				for(int i=0; i<v.size(); i++)
				{
					alocator->free_coin(v[i]);
				}
				v.clear();
			}

			void sort()
			{
				if(!v.empty())
				{
					qsort(&v[0], v.size(), sizeof(Coin*), &Coin::compare);
				}
			}

			void swap(CoinRow & _r)
			{
				v.swap(_r.v);
			}
			
			void add(HUFFMAN_TABLE_ITEM _table[], int _sz, float _total)
			{
				for(int sym=0; sym<_sz; sym++)
				{
					HUFFMAN_TABLE_ITEM & item = _table[sym];
					if(item.freq)
					{
						Coin * p = alocator->alloc_coin();
						p->init(alocator, sym);
						p->value = float(item.freq) / _total;
						v.push_back(p);
					}
				}
				sort();
			}

			void merge(CoinRow * _from)
			{
				E_ASSERT(v.empty());
				int sza = _from->v.size();
				for(int i=0; i<sza-1; i+=2)
				{
					Coin * & p0 = _from->v[i];
					Coin * & p1 = _from->v[i+1];
					p0->merge(p1);
					alocator->free_coin(p1);
					v.push_back(p0);
					p0 = 0;
					p1 = 0;
				}
				_from->clear();
			}


			~CoinRow()
			{
				clear();
			}
		};

		void make_lengths_from_freqs(HUFFMAN_TABLE_ITEM _table[], int _sz, int _max_len)
		{
			E_ASSERT(_sz > 0);
			int used_symbol_count = 0;
			float total = 0;
			for(int sym=0; sym<_sz; sym++)
			{
				HUFFMAN_TABLE_ITEM & item = _table[sym];
				E_ASSERT(item.len == 0);
				if(item.freq)
				{
					total+= item.freq;
					used_symbol_count++;
				}
			}

			if(used_symbol_count == 1)
			{
				for(int i=0; i<_sz; i++)
				{
					if(_table[i].freq)
					{
						_table[i].len = 1;
						break;
					}
				}
			}
			else
			{
				int cur = 0;
				CoinRow row[2];
				row[0].alocator = this;
				row[1].alocator = this;
				for(int len = 1; len <= _max_len; len++)
				{
					CoinRow *a = &row[cur];
					CoinRow *b = &row[1 - cur];
					a->add(_table, _sz, total);
					b->merge(a);
					cur = 1 - cur;
				}

				CoinRow * a = &row[cur];
				a->sort();
				int n0 = a->v.size();
				int n1 = used_symbol_count-1;
				E_ASSERT(n0 == n1);
				int n = n0 < n1 ? n0 : n1;
				for(int i=0; i<n; i++)
				{
					Coin * p = a->v[i];
					for(Coin::Node * node = p->symbols; node != 0; node=node->next)
					{
						int sym = node->s;
						_table[sym].len++;
						E_ASSERT(_table[sym].len <= _max_len);
					}
				}
			}
//#	ifdef NB_DEBUG
//			for(int i=0; i<_sz; i++)
//			{
//				E_ASSERT(!(_table[i].freq>0 && _table[i].len==0));
//			}
//#	endif

		}
#endif
		void make_huffman_table_from_lengths(HUFFMAN_TABLE_ITEM _table[], int _sz)
		{
			uint32 count[33];
			memset(count, 0, sizeof(count));
			for(int i=0; i<_sz; i++)
			{
				HUFFMAN_TABLE_ITEM & item = _table[i];
				uint32 len = item.len;
				E_ASSERT(len < 33);
				count[len]++;
			}
		
			uint32 code = 0;
			count[0] = 0;
			uint32 next_code[33];
			for(int bits=1; bits < 33; bits++)
			{
				code = (code + count[bits-1]) << 1;
				next_code[bits] = code;
			}

#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
			E_TRACE_LINE(L"ZIP:");
#endif
			for(int i=0; i<_sz; i++)
			{
				uint32 len = _table[i].len;
				if(len)
				{
					uint32 code = next_code[len];
					_table[i].code = nb_reverse_bits(code, len);
#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
					E_TRACE_LINE(string(i) +  L": " + bits_to_str(_table[i].code, len));
#endif
					next_code[len]++;
				}
			}
#ifdef NB_ZIP_DUMP_HUFFMAN_CODES
			E_TRACE_LINE(L"");
#endif
		}

		void output_align_to_byte()
		{
			if(bit_buf_pos & 0x07)
			{
				int bits = (bit_buf_pos / 8 + 1) * 8;
				write_bits(0, bits - bit_buf_pos);
			}
		}

		void flush_remain_output()
		{
			if(out_buf_pos)
			{
				int n = out_buf_pos * 4;
				output_total+= n;
				wf(wc, out_buf, n);
				out_buf_pos = 0;
			}
			if(bit_buf_pos)
			{
				int n = (bit_buf_pos + 7) / 8;
				output_total+= n;
				wf(wc, &bit_buf, n);
				bit_buf_pos= 0;
				bit_buf = 0;
			}
		}

		void write_header() throw(const char *)
		{
			if(header == 1)
			{
				// RFC 1950
				uint32 cm = 8;
				uint32 cminfo = 7;
				uint32 cmf =  ((cminfo) << 4) | cm;
				uint8 check = 31 - (cmf * 256) % 31;
				E_ASSERT(((cmf * 256) + check) % 31 == 0 );

				write_bits(cmf, 8);
				write_bits(check, 8);
			}
			else if(header == 2)
			{
				write_bits(0x1f, 8); //ID1
				write_bits(0x8b, 8); //ID2
				write_bits(8, 8); // CM  = deflate
				write_bits(0, 8); // FLG = none
				write_uint32(0); // MTIME[4] = unkown
				write_bits(4, 8); // XFL = fast method
				write_bits(255, 8); // OS = unkown
			}
			else if(header != 0)
			{
				throw(NB_SRC_LOC "unsupported header format");
			}
		}

		void write_footer()
		{
			E_ASSERT(input_total == src_off+src_end);
			output_align_to_byte();
			if(header == 1)
			{
				uint32 adler =  (adler_b << 16) | adler_a;
				write_bits((adler >> 3*8) & 0xff, 8);
				write_bits((adler >> 2*8) & 0xff, 8);
				write_bits((adler >> 1*8) & 0xff, 8);
				write_bits((adler >> 0*8) & 0xff, 8);
			}
			else if(header == 2)
			{
				crc = crc ^ 0xffffffff;
				write_uint32(crc);
				write_uint32(input_total);
			}
		}

	public:
		void write_uint32(uint32 _buf)
		{
			write_bits(_buf & 0x0000ffff, 16);
			write_bits((_buf >> 16) & 0x0000ffff, 16);
		}
		void write_bits(uint32 _buf, int _len)
		{
		//	E_TRACE(bits_to_str(_buf, _len));
			E_ASSERT(_len > 0 && _len < 32);
			//uint32 _buf_bak = _buf;
			//_buf&=low_bits_mask_32[_len];
			//E_ASSERT(_buf_bak == _buf);
			int room = 32 - bit_buf_pos;
			if(room <= _len)
			{
				out_buf[out_buf_pos++] = bit_buf | (_buf << bit_buf_pos);
				bit_buf = _buf >> room;
				bit_buf_pos = _len - room;
				if(out_buf_pos == 8)
				{
					int n = 8 * 4;
					output_total+= n;
					wf(wc, out_buf, n);
					out_buf_pos = 0;
					//E_TRACE_LINE(L"");
					//E_TRACE_LINE(L"FLUSH: ");
					//for(int i=0; i<8; i++)
					//{
					//	E_TRACE(bits_to_str(out_buf[i], 32) + L" ");
					//}
					//E_TRACE_LINE(L"");
				}
			}
			else
			{
				bit_buf|= _buf << bit_buf_pos;
				bit_buf_pos+= _len;
			}
		}

		void update_checksum(uint8 * _p, uint8 * _end)
		{
			switch(header)
			{
			case 1:
				while(_p != _end)
				{
					adler_a = adler_a + *_p++;
					if(adler_a >  65521)
					{
						adler_a-= 65521;
					}
					adler_b = adler_b + adler_a;
					if(adler_b > 65521)
					{
						adler_b-= 65521;
					}
				}
				break;
			case 2:
				E_ASSERT(header == 2);
				while(_p != _end)
				{
					nb_crc32(crc, *_p++);
				}
				break;
			}
		}


		void fetch_more_input()
		{
			if(src_end < SRC_END_INF)
			{
				return;
			}

			E_ASSERT(src_pos >= SRC_CACHE_LINE);
			memmove(src, src + SRC_CACHE_SHIFT, SRC_SIZE - SRC_CACHE_SHIFT);
			src_pos-=SRC_CACHE_SHIFT;
			src_off+=SRC_CACHE_SHIFT;
			if(src_end < SRC_END_INF)
			{
				src_end-= SRC_CACHE_SHIFT;
				E_ASSERT(src_end >= 0);
			}
			else
			{
				uint8 * p = src + SRC_SIZE - SRC_CACHE_SHIFT;
				int n = rf(rc, p, SRC_CACHE_SHIFT);
				uint8* end = p + n;
				update_checksum(p, end);
				input_total+= n;
				if(n < SRC_CACHE_SHIFT)
				{
					src_end = end - src;
				}
			}
		}

		//uint8 read_no_cache(int _offset)
		//{
		//	int index = src_pos + _offset;
		//	return index >= src_end ? 256 : src[index];
		//}

		uint8 read_no_check(int _offset)
		{
			return src[src_pos + _offset];
		}

		int read(int _offset)
		{
			if(src_pos >= SRC_CACHE_LINE)
			{
				fetch_more_input();
			}

			int index = src_pos + _offset;

			if(index >= src_end)
			{
				return 256;
			}
			
			return src[index];
		}

		void init_input()
		{
			if(header == 2)
			{
				crc = 0xffffffff;
			}

			input_total = rf(rc, src, SRC_SIZE);
			update_checksum(src, src+input_total);
			if(input_total < SRC_SIZE)
			{
				src_end = input_total;
			}
		}

		void run() throw(const char *)
		{
			init_input();

			if(read(0) == 256)
			{
				throw(NB_SRC_LOC "no input");
			}

			write_header();

			do
			{
				lz77_encode_block();
				huffman_encode_block();
			}while(src_pos < src_end);

			write_footer();

			flush_remain_output();

			//E_TRACE_LINE(L"[nb] Zip block_count = " + string(block_count)
			//	+ L"\n\tCompress ratio: " + string(float(output_total) * 100 / float(input_total)) + L"%"
			//	+ L"\n\t" + BytesToMKBText(input_total) + L" => " +  BytesToMKBText(output_total)
			//	);
		}

		struct NB_ZIP_HASH_KEY
		{
			uint32 hash;
			union
			{
				uint8 v[4];
				uint32 i;
			};

			void calc_h()
			{
				hash =	_hash_noise[0][v[0]] 
					^	_hash_noise[1][v[1]] 
					^	_hash_noise[2][v[2]];
				hash&= NB_ZIP_HASH_KEY_MASK;
			}
		};

		struct NB_ZIP_HASH_ROOM
		{
			int pos;
			uint32 exact;
		};

		struct NB_ZIP_HASH_TABLE
		{
			int next;
			NB_ZIP_HASH_ROOM nodes[NB_ZIP_HASH_ROOM_COUNT];
		};

		NB_ZIP_HASH_TABLE lz77_hash[NB_ZIP_HASH_TABE_SIZE];

		void lz77_update_hash(const NB_ZIP_HASH_KEY & k)
		{
			NB_ZIP_HASH_TABLE & slot = lz77_hash[k.hash];
			NB_ZIP_HASH_ROOM & node1 = slot.nodes[slot.next++];
			slot.next&= NB_ZIP_HASH_ROOM_COUNT_MASK;
			node1.exact = k.i;
			node1.pos = src_off + src_pos + 1;
		}

		void lz77_update_hash(int _offset)
		{
			int pos0 = src_pos + _offset;
			int pos = src_off + pos0;

			NB_ZIP_HASH_KEY k;
			k.v[0] = src[pos0+0];
			k.v[1] = src[pos0+1];
			k.v[2] = src[pos0+2];
			k.v[3] = 0;
			k.calc_h();
			NB_ZIP_HASH_TABLE & slot = lz77_hash[k.hash];
			NB_ZIP_HASH_ROOM & node1 = slot.nodes[slot.next++];
			slot.next&= NB_ZIP_HASH_ROOM_COUNT_MASK;
			node1.exact = k.i;
			node1.pos = pos + 1;
		}

		static int calc_weight(int len, int off)
		{
			int extra = _length_extra_table[_length_index[len]] + _distance_extra_table[_distance_index[-off]];
			return (len-1) * 8 - extra;
		}

		struct SOL
		{
			int sym;
			int off;
			int len;
		};

		SOL lz77_find_match(const NB_ZIP_HASH_KEY & k)
		{
			SOL result;
			result.len = NB_ZIP_MIN_WORD_LEN-1;
			int result_weight = -99999;
			NB_ZIP_HASH_TABLE & slot = lz77_hash[k.hash];

			int min_offset = -src_pos;
			if(min_offset < -32767)
			{
				min_offset = -32767;
			}
			const int * order = hash_node_lookup_order[slot.next];
			for(int o = 0; o<NB_ZIP_HASH_ROOM_COUNT; o++)
			{
				NB_ZIP_HASH_ROOM & node = slot.nodes[order[o]];
				if(node.pos == 0 || node.exact != k.i)
				{
					continue;
				}
				int real_pos = node.pos - src_off - 1;
				int real_offset = real_pos - src_pos;

				if(real_offset >= min_offset && real_offset <= -1) // -2, -1 is valid, but inefficient 
				{
					int cur_length = NB_ZIP_MIN_WORD_LEN;
					int cur_offset = real_offset;

					if(memcmp(src + src_pos + cur_offset, src + src_pos, 3) != 0)
					{
						continue; // hash conflict
					}
					int byte0, byte1;

					int max_word_size = src_end - src_pos;
					if(max_word_size > NB_ZIP_MAX_WORD_LEN)
					{
						max_word_size = NB_ZIP_MAX_WORD_LEN;
					}
					//int src_pos_bak = src_pos;
					while(cur_length < max_word_size 
						&& (byte0 = read_no_check(cur_offset+cur_length)) 
							== (byte1 = read_no_check(cur_length))
							)
					{
						//E_ASSERT( byte0 != 256);
						cur_length++;
					}

					//E_ASSERT(src_pos_bak == src_pos);
					int cur_weight = calc_weight(cur_length, cur_offset);
					if(cur_weight > result_weight)
					{
						result_weight = cur_weight;
						result.off = cur_offset;
						result.len = cur_length;
						if(cur_length >= NB_ZIP_MAX_WORD_LEN)
						{
							break;
						}
					}
				}
			}

			if(result.len < NB_ZIP_MIN_WORD_LEN)
			{
				result.len = 1;
				result.off = 0;
				result.sym = k.v[0];
			}

			return result;
		}

		struct LZ77_MATCH
		{
			int distance;
			union
			{
				int code;
				int lendth;
			};
		};
		

		void lz77_words_add_lookup(const SOL & _sol)
		{
			//E_ASSERT(lz77_pos < NB_ZIP_BLOCK_SIZE);
			Word & w = lz77_buf[lz77_pos++];
			//E_ASSERT(lz77_pos>=0&&lz77_pos<e::NB_ZIP_BLOCK_SIZE);
			{
				int table_index = _length_index[_sol.len];
				//while(_sol.len >= _length_offset_table[table_index+1])
				//{
				//	table_index++;
				//}
				//E_ASSERT(table_index < sizeof(_length_extra_table) / sizeof(uint32));
				w.length_code = table_index + 257;
				w.length_extra_bits = _length_extra_table[table_index];
				w.length_extra_data = _sol.len - _length_offset_table[table_index];
			}

			literal_huffman_table[w.length_code].freq++;

			{
				int distance = -_sol.off;
				int distance_index = _distance_index[distance];
				//while(distance >= _distance_offset_table[distance_index+1])
				//{
				//	distance_index++;
				//}

				//E_ASSERT(distance_index < sizeof(_distance_extra_table) / sizeof(uint32));
				w.distance_code = distance_index;
				w.distance_extra_bits = _distance_extra_table[distance_index];
				w.distance_extra_data = distance - _distance_offset_table[distance_index];
			}

			//E_ASSERT(w.distance_code < sizeof(distance_huffman_table) / sizeof(HUFFMAN_TABLE_ITEM));
			distance_huffman_table[w.distance_code].freq++;
#ifdef NB_ZIP_DUMP_LZ77
			E_TRACE_LINE(string(lz77_pos) 
				+ L"\t[257+" 
				+ string(w.length_code-257) 
				+ L", " 
				+ string(w.length_extra_bits) 
				+ L", " 
				+ (w.length_extra_bits ? string(w.length_extra_data) : L"NA")
				+ L"] ["
				+ string(w.distance_code) 
				+ L", " 
				+ string(w.distance_extra_bits) 
				+ L", " 
				+ (w.distance_extra_bits ? string(w.distance_extra_data) : L"NA")
				+ L"] ("
				+ string(_sol.len)
				+ L", "
				+ string(_sol.off)
				+ L")" 
				);
#endif
			//E_ASSERT(w.length_code < sizeof(literal_huffman_table) / sizeof(HUFFMAN_TABLE_ITEM));
			src_pos+= _sol.len;
			//E_ASSERT(src_pos <= src_end);
		}

		void lz77_words_add_symbol(int _sym)
		{
			E_ASSERT(_sym >= 0 && _sym < 256);
		//	E_ASSERT(lz77_pos < NB_ZIP_BLOCK_SIZE);
			Word & w = lz77_buf[lz77_pos++];
			w.length_code = _sym;
			literal_huffman_table[w.length_code].freq++;
	//		E_ASSERT(src_pos <= SRC_SIZE);

#ifdef NB_ZIP_DUMP_LZ77
			int input_pos = input_total - src_end + src_pos;
			if(w.length_code >= 32 && w.length_code < 128)
			{
				E_TRACE_LINE(string(lz77_pos) + L"\t'" + string((wchar_t) w.length_code) + L"' @" + string(input_pos));
			}
			else
			{
				E_TRACE_LINE(string(lz77_pos) + L"\t(" + ByteToHex(w.length_code) + L") @" + string(input_pos));
			}
			//E_ASSERT(w.length_code < sizeof(literal_huffman_table) / sizeof(HUFFMAN_TABLE_ITEM));
#endif
			src_pos++;
		//	E_ASSERT(src_pos <= src_end);
		}

		void lz77_words_add_sol(const SOL & _sol)
		{
			if(_sol.off)
			{
				lz77_words_add_lookup(_sol);
			}
			else
			{
				lz77_words_add_symbol(_sol.sym);
			}
		//	E_ASSERT(src_pos <= src_end);
		}

		void lz77_words_add_end()
		{
			E_ASSERT(lz77_pos < NB_ZIP_BLOCK_SIZE);
			Word & w = lz77_buf[lz77_pos++];
			w.length_code = 256;
			E_ASSERT(literal_huffman_table[256].freq == 0);
			literal_huffman_table[256].freq++;
#ifdef NB_ZIP_DUMP_LZ77
			E_TRACE_LINE(string(lz77_pos) + L"\t(BLOCK END)");
#endif
		}

		static int ilog(uint32 x)
		{
			int ret = 0;
			while(x)
			{
				ret++;
				x>>= 1;
			}
			return ret;
		}

		void lz77_encode_block()
		{
			lz77_pos = 0;
			memset(literal_huffman_table, 0, sizeof(literal_huffman_table));
			memset(distance_huffman_table, 0, sizeof(distance_huffman_table));

			NB_ZIP_HASH_KEY ka[NB_ZIP_TRY_MATCH];
			memset(ka, 0, sizeof(ka));
			SOL sa[NB_ZIP_TRY_MATCH];
			const static int LZ77_VEND = NB_ZIP_BLOCK_SIZE - NB_ZIP_TRY_MATCH - 1;
			while(lz77_pos < LZ77_VEND)
			{
				if(src_pos >= SRC_CACHE_LINE)
				{
					fetch_more_input();
				}

				int remain_input = src_end - src_pos;
				if(remain_input < 3)
				{
					for(int i=0; i<remain_input; i++)
					{
						uint8 code = read_no_check(0);
						lz77_words_add_symbol(code);
					}
					break;
				}
				
				{
					NB_ZIP_HASH_KEY & k = ka[0];
					k.v[0] = read_no_check(0);
					k.v[1] = read_no_check(1);
					k.v[2] = read_no_check(2);
					k.calc_h();

					SOL & sol = sa[0];
					sol = lz77_find_match(k);
					if(sol.off == 0)
					{
						if(lz77_pos == LZ77_VEND-1 && remain_input > 1024)
						{
							break;
						}
						lz77_update_hash(k);
						lz77_words_add_symbol(k.v[0]);
						continue;
					}
				}

				int try_match = remain_input - NB_ZIP_TRY_MATCH;
				if(try_match > NB_ZIP_TRY_MATCH)
				{
					try_match = NB_ZIP_TRY_MATCH;
				}

				int select_index = 0;
				int select_save = calc_weight(sa[0].len, sa[0].off);
				int src_pos_bak = src_pos;
				src_pos++;

				for(int tn = 1; tn < try_match; tn++)
				{
					//if(src_pos + 2 >= src_end)
					//{
					//	break;
					//}

					NB_ZIP_HASH_KEY & k = ka[tn];
					k.v[0] = read_no_check(0);
					k.v[1] = read_no_check(1);
					k.v[2] = read_no_check(2);
					k.calc_h();

					SOL & sol = sa[tn];
					sol = lz77_find_match(k);
					int current_save_bits;
					if(sol.off)
					{
						current_save_bits = calc_weight(sol.len, sol.off);
					}
					else
					{
						current_save_bits = 0;
					}

					if(current_save_bits > select_save)
					{
						select_save = current_save_bits;
						select_index = tn;
					}

					src_pos++;
				}

				src_pos = src_pos_bak;

				for(int i=0; i<select_index; i++)
				{
					lz77_update_hash(ka[i]);
					lz77_words_add_symbol(read_no_check(0));
				}
				{
					SOL & sol = sa[select_index];
					for(int i=0; i<sol.len; i++)
					{
						lz77_update_hash(i);
					}
					lz77_words_add_sol(sol);
				}
			}

			lz77_words_add_end();
		}

#ifndef NB_ZIP_STATIC_BLOCK_ONLY
		void make_and_write_dynamic_huffman()
		{
			for(literal_code_count = 286; literal_code_count>0; literal_code_count--)
			{
				if(literal_huffman_table[literal_code_count-1].freq)
				{
					break;
				}
			}
			E_ASSERT(literal_code_count);
			make_lengths_from_freqs(literal_huffman_table, literal_code_count, 15);

			for(real_distance_code_count = 30; real_distance_code_count>0; real_distance_code_count--)
			{
				if(distance_huffman_table[real_distance_code_count-1].freq)
				{
					break;
				}
			}

			if(real_distance_code_count == 0)
			{
				fake_distance_code_count = 1;
			}
			else
			{
				fake_distance_code_count = real_distance_code_count;
			}

			if(real_distance_code_count)
			{
				make_lengths_from_freqs(distance_huffman_table, real_distance_code_count, 15);
			}

			//E_TRACE_LINE("");
			int cl_count = literal_code_count + fake_distance_code_count;
			uint8 a[288+32];
			for(int i=0; i<literal_code_count; i++)
			{
				a[i] = literal_huffman_table[i].len;
				E_ASSERT(a[i] <= 15);
			//	E_TRACE_LINE(L"a[" + string(i) + L"] = " + string(a[i]));
			}
			if(real_distance_code_count)
			{
				for(int i=literal_code_count; i<cl_count; i++)
				{
					a[i] = distance_huffman_table[i-literal_code_count].len;
					E_ASSERT(a[i] <= 15);
				//	E_TRACE_LINE(L"a[" + string(i) + L"] = " + string(a[i]));
				}
			}
			else
			{
				a[literal_code_count] = 0;
			}

			HUFFMAN_TABLE_ITEM cl_huffman_table[19];
			memset(cl_huffman_table, 0, sizeof(cl_huffman_table));
			uint8 max_rl_cl_symbol = 0;

			int rl_cl_count = 0; // run length encode cl count
			for(int i=0; i < cl_count;)
			{
				int &j = rl_cl_count;
				uint8 ch = a[i];
				int times = 1;
				for(int k=i+1; k<cl_count && a[k] == a[i]; k++)
				{
					times++;
				}

				//if(ch == 0 && i+times == cl_count)
				//{
				//	break;
				//}

				if(ch == 0 && times >= 11)
				{
					if(times > 138)
					{
						times = 138;
					}
					a[j++] = 18;
					a[j++] = times - 11;
					if(max_rl_cl_symbol < 18)
					{
						max_rl_cl_symbol = 18;
					}
					cl_huffman_table[18].freq++;
				}
				else if(ch == 0 && times >= 3)
				{
					a[j++] = 17;
					a[j++] = times - 3;
					if(max_rl_cl_symbol < 17)
					{
						max_rl_cl_symbol = 17;
					}
					cl_huffman_table[17].freq++;
				}
				else if(times >= 4)
				{
					if(times >= 7)
					{
						times = 6;
					}
					a[j++] = ch;
					a[j++] = 16;
					a[j++] = times - 4;
					if(max_rl_cl_symbol < 16)
					{
						max_rl_cl_symbol = 16;
					}
					cl_huffman_table[ch].freq++;
					cl_huffman_table[16].freq++;
				}
				else
				{
					for(int k=0; k<times; k++)
					{
						a[j++] = ch;
					}

					if(max_rl_cl_symbol < ch)
					{
						max_rl_cl_symbol = ch;
					}
					cl_huffman_table[ch].freq+= times;
				}
				i+= times;			
			}


			make_lengths_from_freqs(cl_huffman_table, max_rl_cl_symbol+1, 7);

			//E_TRACE_LINE("");
			//for(int i=0; i < max_rl_cl_symbol+1; i++)
			//{
			//	E_TRACE_LINE(L"len[" + string(i) + L"] = " + string(cl_huffman_table[i].len));
			//}

			int write_clcl_count = 0;
			uint8 clcl[19] = {0};
			for(int i=0; i<19; i++)
			{
				int len = cl_huffman_table[_clcl_table[i]].len;
				if(len)
				{
					clcl[i] = len;
					write_clcl_count = i+1;
				}
			}

			uint32 HLIT = literal_code_count - 257;
			uint32 HDIST = fake_distance_code_count - 1;
			uint32 HCLEN = write_clcl_count - 4;
			write_bits(HLIT, 5);
			write_bits(HDIST, 5);
			write_bits(HCLEN, 4);

			for(int i = 0; i<write_clcl_count; i++)
			{
				write_bits(clcl[i], 3);
			}

			make_huffman_table_from_lengths(cl_huffman_table, max_rl_cl_symbol+1);
			for(int i=0; i<rl_cl_count;)
			{
				int c = a[i++];
				HUFFMAN_TABLE_ITEM * p = cl_huffman_table + c;
				write_bits(p->code, p->len);
				switch(c)
				{
				case 18:
					write_bits(a[i++], 7);
					break;
				case 17:
					write_bits(a[i++], 3);
					break;
				case 16:
					write_bits(a[i++], 2);
					break;
				}
			}

			make_huffman_table_from_lengths(literal_huffman_table, literal_code_count);

		//	if(fake_distance_code_count > 0)
			{
				make_huffman_table_from_lengths(distance_huffman_table, fake_distance_code_count);
			}
		}

#else
		void make_static_huffman()
		{
			// static
			for(int i=0; i<=143; i++)
			{
				literal_huffman_table[i].len = 8;
			}

			for(int i=144; i<=255; i++)
			{
				literal_huffman_table[i].len = 9;
			}

			for(int i=256; i<=279; i++)
			{
				literal_huffman_table[i].len = 7;
			}

			for(int i=280; i<288; i++)
			{
				literal_huffman_table[i].len  = 8;
			}

			for(int i=0; i<32; i++)
			{
				distance_huffman_table[i].len = 5;
			}

			make_huffman_table_from_lengths(literal_huffman_table, 288);
			literal_code_count = 288;
			make_huffman_table_from_lengths(distance_huffman_table, 32);
			fake_distance_code_count = 32;
		}
#endif

		void huffman_encode_block()
		{
			literal_code_count = 0;
			fake_distance_code_count = 0;

			int is_final = read(0) == 256;

#ifndef NB_ZIP_STATIC_BLOCK_ONLY
			write_bits(is_final, 1);
			write_bits(2, 2);
			make_and_write_dynamic_huffman();
#else
			write_bits(is_final, 1);
			write_bits(1, 2);
			make_static_huffman();
#endif

			for(int i=0; i<lz77_pos; i++)
			{
				Word & w = lz77_buf[i];
				
				// output w.length_code;
				{
					uint32 sym = w.length_code;
					char tmp = (char) sym;
					E_ASSERT(sym>=0 && sym <literal_code_count);
					HUFFMAN_TABLE_ITEM * p = literal_huffman_table + sym;
					write_bits(p->code, p->len);
				}

				if(w.length_code > 256)
				{
					E_ASSERT(real_distance_code_count != 0);
					if(w.length_extra_bits)
					{
						E_ASSERT(w.length_extra_bits<=5);
						E_ASSERT(w.length_extra_data<32);
						write_bits(w.length_extra_data, w.length_extra_bits);
					}

					// output w.distance_code
					uint32 dis = w.distance_code;
					E_ASSERT(dis>=0 && dis<fake_distance_code_count);
					HUFFMAN_TABLE_ITEM * p = distance_huffman_table + dis;
					write_bits(p->code, p->len);

					if(w.distance_extra_bits)
					{
						E_ASSERT(w.distance_extra_bits<=13);
						E_ASSERT(w.distance_extra_data< (1<<13));
						write_bits(w.distance_extra_data, w.distance_extra_bits);
					}
				}
			}
			block_count++;
		}
	};

	void nb_zip(void *_rc,
		int (*_rf)(void*, void*, unsigned int),
		void *_wc,
		int (*_wf)(void*, void*, unsigned int),
		int _header)
	{
		// NB_PROFILE_INCLUDE;
		if( _rf == 0 || _wf == 0 ||	_header <0 || _header >2)
		{
			throw(NB_SRC_LOC "bad param");
		}

		Zip * ctx = enew Zip(_rc, _rf, _wc, _wf, _header);
		try
		{
			ctx->run();
			delete ctx;
		}
		catch(const char * _exp)
		{
			delete ctx;
			throw(_exp);
		}
	}

	// 内存 => 内存
	struct NB_ZIOBUF
	{
		uint8 * src;
		size_t in_num;

		uint8 * out_buf;
		size_t out_buf_size;
		size_t out_num;
		static int read(void * _io, void * _buf, unsigned int _n)
		{
			NB_ZIOBUF * io = (NB_ZIOBUF*) _io;
			int n = _n <= io->in_num ? _n : io->in_num;
			memcpy(_buf, io->src, n);
			io->in_num-= n;
			io->src+= n;
			return n;
		}

		static int write(void * _io, void * _buf, unsigned int _n)
		{
			NB_ZIOBUF * io = (NB_ZIOBUF*) _io;
			if(io->out_num + _n > io->out_buf_size)
			{
				io->out_buf_size = io->out_num + _n;
				io->out_buf_size = io->out_buf_size + (io->out_buf_size >> 1) + 1;
				io->out_buf = (uint8*) realloc(io->out_buf, io->out_buf_size);
			}
			memcpy(io->out_buf + io->out_num, _buf, _n);
			io->out_num+= _n;
			return _n;
		}
	};

	void nb_block_unzip(void * & _out_buf, size_t & _out_len, void * _src, size_t _in_len) throw(const char *)
	{
		// NB_PROFILE_INCLUDE;
		_out_buf = 0;
		_out_len = 0;

		if(_in_len == 0)
		{
			throw(NB_SRC_LOC "no input");
		}

		NB_ZIOBUF io;
		io.src = (uint8 *)_src;
		io.in_num = _in_len;
		io.out_buf_size = _in_len * 2 + 2;
		io.out_buf = (uint8 *)malloc(io.out_buf_size);
		io.out_num = 0;

		try
		{
			nb_unzip(&io, &NB_ZIOBUF::read, &io, &NB_ZIOBUF::write);
		}
		catch(const char * _s)
		{
			free(io.out_buf);
			throw(_s);
		}
		_out_buf = io.out_buf;
		_out_len = io.out_num;
	}

	void nb_block_zip(void * & _out_buf, size_t & _out_len, void * _src, size_t _in_len, int _header) throw(const char *)
	{
		// NB_PROFILE_INCLUDE;
		_out_buf = 0;
		_out_len = 0;

		if(_in_len == 0)
		{
			throw(NB_SRC_LOC "no input");
		}

		NB_ZIOBUF io;
		io.src = (uint8 *)_src;
		io.in_num = _in_len;
		io.out_buf_size = _in_len * 2 + 2;
		io.out_buf = (uint8 *)malloc(io.out_buf_size);
		io.out_num = 0;

		try
		{
			nb_zip(&io, &NB_ZIOBUF::read, &io, &NB_ZIOBUF::write, _header);
		}
		catch(const char * _s)
		{
			free(io.out_buf);
			throw(_s);
		}
		catch(...)
		{
			E_ASSERT(0);
		}
		_out_buf = io.out_buf;
		_out_len = io.out_num;
	}


	//void nb_block_zip_old(void * & _out_buf, size_t & _out_len, void * _src, size_t _in_len, int _header) throw(const char *)
	//{
	//	if(_header != 1)
	//	{
	//		throw(NB_SRC_LOC "unsupported format");
	//	}

	//	int blocks = (_in_len + 32767) / 32768;
	//	_out_len = 2 //header
	//		+ blocks // block types
	//		+ blocks * 4 // block lenss
	//		+ _in_len // data
	//		+ 4; // adler
	//	_out_buf = malloc(_out_len);
	//	uint8 * dst = (uint8*) _out_buf;
	//	uint8 * padler = dst+_out_len-4;

	//	uint32 cm = 8;
	//	uint32 cminfo = 7;
	//	uint32 cmf =  ((cminfo) << 4) | cm;
	//	*dst++ = cmf;
	//	uint8 check = 31 - (cmf * 256) % 31;
	//	E_ASSERT(((cmf * 256) + check) % 31 == 0 );
	//	*dst++ = check;

	//	uint8 * src = (uint8*) _src;
	//	size_t len = _in_len;
	//	for(int i=0; i<blocks; i++)
	//	{
	//		*dst++ = (i== blocks-1) ? 1 : 0;
	//		size_t len1 = len > 32768 ? 32768 : len;
	//		*dst++ = (uint8)(len1 & 0xff);
	//		*dst++ = (uint8)((len1 >> 8) & 0xff);
	//		*dst++ = ~((uint8)(len1 & 0xff));
	//		*dst++ = ~((uint8)((len1 >> 8) & 0xff));
	//		memcpy(dst, src, len1);
	//		dst+= len1;
	//		src+= len1;
	//		len-= len1;
	//	}

	//	// *dst++ = 0x07;
	//	if(padler != dst)
	//	{
	//		throw(NB_SRC_LOC "internal error");
	//	}
	//	uint32 adler = adler32((uint8*)_src, _in_len);
	//	padler[0] = (adler >> 3*8) & 0xff;
	//	padler[1] = (adler >> 2*8) & 0xff;
	//	padler[2] = (adler >> 1*8) & 0xff;
	//	padler[3] = (adler >> 0*8) & 0xff;
	//}
}
