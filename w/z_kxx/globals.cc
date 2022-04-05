
// #include "../config.h"
#include <nbug/core/debug.h>
#include <z_kxx/globals.h>
#include <nbug/tl/map.h>
#include <nbug/tl/array.h>
#include <nbug/core/env.h>
#include <nbug/core/file.h>

namespace e
{
	const RGBA & hue_color(int _hue)
	{
		static const RGBA _hue_table[12] = 
		{
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 0.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 0.0f, 1.0f, 1.0f},
			{0.5f, 0.5f, 0.0f, 1.0f},
			{0.0f, 0.5f, 0.5f, 1.0f},
			{0.5f, 0.0f, 0.5f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
		};
		E_ASSERT(_hue >= 0 && _hue <12);
		return _hue_table[_hue];
	}

	static uint32 g_randomSeed = 1;


#ifdef NB_DEBUG

	struct SRCLOC
	{
		int line;
		const char * file;
		bool operator<(const SRCLOC & _r) const
		{ return line == _r.line ? file < _r.file : line < _r.line; }
		bool operator==(const SRCLOC & _r) const
		{ return line == _r.line && file == _r.file; }
	};
	static Map<SRCLOC, uint32> * g_src_loc_map = 0;
	static uint32 g_next_src_loc_id = 0;
	static Array<SRCLOC> * g_src_loc_array = 0;
	struct RANDOM_ORDER_ITEM
	{
		uint32 src_loc;
		uint32 seed;
	};
	static Array<RANDOM_ORDER_ITEM> * g_random_order = 0;

	void dbg_logic_random_dump()
	{
		int n = 0;
		Path path;
		do
		{
			path = Env::GetDataFolder() |  ( L"rnd_dump_" + string(n) + L".txt");
			n++;
		}while(FS::IsFile(path));
		FileRef f = FS::OpenFile(path, true);
		if(f)
		{
			for(int i=0; i<g_random_order->size(); i++)
			{
				f->WriteLine(stringa((*g_random_order)[i].seed) + L"\t" + string((*g_random_order)[i].src_loc));
			}

			f->WriteLine("");

			for(int i=0; i<g_src_loc_array->size(); i++)
			{
				f->WriteLine(stringa(i) + L"\t" + string((*g_src_loc_array)[i].line) + L"\t" + string((*g_src_loc_array)[i].file));
			}
		}
	}

	static void dbg_logic_random_cleanup()
	{
		if(!g_random_order->empty())
		{
			dbg_logic_random_dump();
		}
		delete g_src_loc_map;
		g_src_loc_map = 0;
		delete g_random_order;
		g_random_order = 0;
		delete g_src_loc_array;
		g_src_loc_array = 0;
	}

	static void dbg_logic_random_init()
	{
		atexit(&dbg_logic_random_cleanup);
		g_random_order = enew Array<RANDOM_ORDER_ITEM>();
		g_src_loc_map = enew Map<SRCLOC, uint32>();
		g_src_loc_array = enew Array<SRCLOC>();
	}
	
	static void dbg_logic_random_record(const char * _file, int _line, uint32 _seed)
	{
		SRCLOC sl;
		sl.file = _file;
		sl.line = _line;
		Map<SRCLOC, uint32>::iterator it = g_src_loc_map->find(sl);
		uint32 src_loc_id;
		if(it == g_src_loc_map->end())
		{
			(*g_src_loc_map)[sl] = g_next_src_loc_id;
			src_loc_id = g_next_src_loc_id;
			g_src_loc_array->push_back(sl);
			g_next_src_loc_id++;
		}
		else
		{
			src_loc_id = it->second;
		}

		RANDOM_ORDER_ITEM item;
		item.seed = _seed;
		item.src_loc = src_loc_id;
		g_random_order->push_back(item);
	}

	void dbg_logic_random_set_seed(uint _seed, const char * _file, int _line)
	{
		// NB_PROFILE_INCLUDE;
		if(!g_random_order)
		{
			dbg_logic_random_init();
		}

		if(!g_random_order->empty())
		{
			dbg_logic_random_dump();
			g_random_order->clear();
		}

		g_randomSeed = (_seed ^ (_seed >> 16)) & 0xffff;
		dbg_logic_random_record(_file, _line, g_randomSeed);
	}

	int dbg_logic_random(const char * _file, int _line)
	{
		if(!g_random_order)
		{
			dbg_logic_random_init();
		}

		g_randomSeed = (uint)(g_randomSeed * 1103515245 + 12345) % 32768;
		dbg_logic_random_record(_file, _line, g_randomSeed);
		return g_randomSeed;
 	}

	float dbg_logic_random_float(const char * _file, int _line)
	{
		return float(dbg_logic_random(_file, _line)) / 32768;
	}

#else
	int logic_random_int()
	{
		g_randomSeed = (uint)(g_randomSeed * 1103515245 + 12345) % 32768;
		return g_randomSeed;
 	}
	float logic_random_float()
	{
		return float(logic_random_int()) / 32768;
	}
	void logic_random_set_seed(uint _seed)
	{
		// NB_PROFILE_INCLUDE;
		_seed = (_seed ^ (_seed >> 16)) & 0xffff;
		g_randomSeed = _seed;
	}
#endif

	uint32 logic_random_get_seed()
	{
		return g_randomSeed;
	}


}
