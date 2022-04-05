#ifndef NB_CORE_HUFFMAN_H
#define NB_CORE_HUFFMAN_H

#include <nbug/core/debug.h>

namespace e
{
	uint32 nb_reverse_bits(uint32 _src, uint32 _bit_count);

	struct SLC;
	class DecodeHuffman32
	{
	protected:

		struct TB
		{
			enum
			{
				typeUnused,
				typeTable,
				typeSymbol,
			};
			uint8 type;
			uint8 len;
			union
			{
				struct
				{
					uint32 sym;
				};
				struct
				{
					TB * next;
				};
			};

			void Delete()
			{
				if(type == typeTable)
				{
					for(char i=0; i<len; i++)
					{
						(next+i)->Delete();
					}
					free(next);
				}
				free(this);
			}
		};

		uint8 min_len;
		uint8 max_len;
		uint8 root_len;
		TB * root;

		static void free_table_block(TB * & _table, uint8 _len);
		static bool add_to_table(TB * _table, uint8 _table_len, SLC * _slc, SLC * _end, uint8 _cut);
	public:
#ifdef NB_DEBUG
		void DumpTable(TB * table = 0,  uint8 len = 0, int _indent = 0);
#endif
		DecodeHuffman32();
		~DecodeHuffman32();
		// _codetype: 0=zip, 1=vorbis
		bool Init(uint8 * _lengths, size_t _sym_count, int _codetype);

		template <typename _CTX>
		inline uint32 Decode(_CTX * _ctx)
		{
			uint8 len = root_len;
			uint32 code = _ctx->read_bits(len);
			TB * entry = root + code;
			while(entry->type == TB::typeTable)
			{
				_ctx->drop_bits(len);
				len = entry->len;
				code = _ctx->read_bits(len);
				E_ASSERT(entry->next);
				entry = entry->next + code;
			}
			if(entry->type)
			{
				_ctx->drop_bits(entry->len);
				return entry->sym;
			}
			else
			{
				throw(NB_SRC_LOC "huffman decode mismatch.");
			}
		}
	};

}


#endif
