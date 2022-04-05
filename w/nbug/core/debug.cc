
// #include "private.h"
#ifdef NB_WINDOWS
#	include <windows.h>
#endif

#ifdef NB_LINUX
#	include <pthread.h>
#	include <errno.h>
#endif

#include <memory.h>
#include <stdio.h>

#include <nbug/core/debug.h>
#include <nbug/core/obj.h>
#include <nbug/core/def.h>
#include <nbug/core/thread.h>
#include <nbug/core/file.h>
#include <nbug/core/env.h>
#include <nbug/core/time.h>

#ifdef _MSC_VER
#	define __thread __declspec(thread)
#endif

namespace e
{
	//bool nb_enable_write_log = true;


	void throwf(const char * _fmt, ...) throw(const char *)
	{
		static const int NB_EXCEPTION_BUF_SIZE = 1024;
		static __thread char _exception_buf[NB_EXCEPTION_BUF_SIZE];
		va_list argptr;
		va_start(argptr, _fmt);
#ifdef _MSC_VER
		vsprintf_s(_exception_buf, NB_EXCEPTION_BUF_SIZE, _fmt, argptr);
#else
		vsnprintf(_exception_buf, NB_EXCEPTION_BUF_SIZE, _fmt, argptr);
#endif
		va_end(argptr);
		throw(_exception_buf);
	}

	void throws(const stringa & _s) throw(const char *)
	{
		throwf("%s", _s.c_str());
	}

	void throw_posix(const char * _prefix) throw(const char *)
	{
		const char * s = strerror(errno);
		throwf("%s: %s", _prefix, s);
	}

	void throw_winapi(const char * _prefix) throw(const char *)
	{
#ifdef NB_WINDOWS
		char buf[512];
		::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
			, 0
			, GetLastError()
			, LANG_USER_DEFAULT
			, buf
			, 512
			, 0);
		throwf("%s: %s", _prefix, buf);
#else
		E_ASSERT1(0, "throw_winapi() is only avaible on windows.");
#endif
	}

	void write_log(const string & _message)
	{
		//E_TRACE_LINE(_message);
		Path path(Env::GetDataFolder() | (Env::GetShortName() + L".log"));
		bool init = !FS::IsFile(path);
		FileRef file = FS::OpenFile(path, true);
		if(!file)
		{
			return;
		}
		if(init)
		{
			file->Write("\xEF\xBB\xBF", 3);
		}

		if(!file->Seek(0, SEEK_END))
		{
			return;
		}
		static const int LOG_KEEP_SIZE = 15000;
		static const int LOG_MAX_SIZE = 20000;
		int64 len = file->Tell();
		if(len > LOG_MAX_SIZE)
		{
			file->Seek(LOG_MAX_SIZE - LOG_KEEP_SIZE, SEEK_SET);
			char buf1[101];
			bool found_line_end = false;
			int64 pos = LOG_MAX_SIZE - LOG_KEEP_SIZE;
			while(!found_line_end)
			{
				if(!file->Read(buf1, 100))
				{
					break;
				}
				char * p = strchr(buf1, '\n');
				if(p)
				{
					pos+= p - buf1;
					found_line_end = true;
					break;
				}
				else
				{
					pos+= 100;
				}
			}
			if(found_line_end)
			{
				file->Seek(pos+1, SEEK_SET);
				int n = (int)(len - pos - 1);
				if(n > 0)
				{
					char * buf = enew char[n];
					file->Read(buf, n);
					file->SetSize(0);
					file->Write("\xEF\xBB\xBF", 3);
					file->Write(buf, n);
					delete[] buf;
				}
				else
				{
					file->Seek(0, SEEK_END);
				}
			}
			else
			{
				file->Seek(0, SEEK_END);
			}
		}

		file->WriteLine(stringa(_message, CHARSET_UTF8));
	}

	void message(const string & _msg)
	{
#	if defined(NB_WINDOWS) && defined(_MSC_VER) && defined(NB_DEBUG)
        OutputDebugString(_msg.c_str());
        OutputDebugString(L"\n");
#	endif

		{
			stringa a(_msg);
			fputs(a.c_str(), stdout);
			fputs("\n", stdout);
			fflush(stdout);
		}

		write_log(_msg);
	}

	//void warning(const Char * module, const string & _msg)
	//{
	//	string s;
	//	if(module)
	//	{
	//		s = L"[" + string(module) + L"] (WW) ";
	//	}
	//	else
	//	{
	//		s = L" (WW) ";
	//	}
	//	
	//	s+= _msg;

	//	write_log(s);
	//	//if(!_extra.empty())
	//	//{
	//	//	write_log(_extra);
	//	//}
	//}

    void error(const string & _msg)
    {
		message(_msg);

#ifdef NB_WINDOWS
		::MessageBox(NULL, _msg.c_str(), Env::GetShortName().c_str(), MB_OK | MB_ICONERROR);
#else
	//	fputs("*ERROR* ", stdout);
		stringa a(_msg);
		fputs(a.c_str(), stdout);
		fputs("\n", stdout);
		fflush(stdout);

		// add backslashes for command line
		static char _sls[] = "\\";
		char src[2] =  {'?', 0};
		char rep[3] = {'\\', '?', 0};
		for(int i=0; i<sizeof(_sls)-1; i++)
		{
			src[0] = _sls[i];
			rep[1] = _sls[i];
			a.replace(src, rep);
		}

		stringa cmd;
		static int rc;

		bool has_gxmessage = system("which gxmessage") == 0;
		bool has_xmessage = system("which xmessage") == 0;

		if(has_gxmessage || has_xmessage)
		{

			cmd = stringa(has_gxmessage ? "gxmessage" : "xmessage") +
					" -title \'" + Env::GetShortName() + "\' -fg red *ERROR* \'" + a + "\'";
			rc = system(cmd.c_str());
			return;
		}

		bool has_notify_send = system("which notify-send") == 0;
		if(has_notify_send)
		{
			cmd = "notify-send -u critical --icon=\"error\" \'[" + Env::GetShortName() + "] " + a + "\'";
			rc = system(cmd.c_str());
			return;
		}

#endif
    }

	bool is_debugger_present()
	{
#	if defined(NB_WINDOWS)
		return IsDebuggerPresent() ? true : false;
#	else
		return false;
#	endif

	}

#ifdef NB_DEBUG

    void debug_write_line(const string & _msg);

    void assert_failed(const string & _msg, const string & _file, int _line)
    {
		string message =  _file + L"(" + string(_line) + L"): " + _msg;
        debug_write_line(message);
		if(!is_debugger_present())
		{
			error(message);
		}
    }

    void debug_write_line(const string & _msg)
    {
    	//global_mutex.Lock();
#	if defined(NB_WINDOWS) && defined(_MSC_VER)
        OutputDebugString(_msg.c_str());
        OutputDebugString(L"\n");
#	endif

		stringa a(_msg);
		fputs(a.c_str(), stdout);
		fputs("\n", stdout);
		fflush(stdout);

		//global_mutex.Unlock();
    }

	void debug_write(const string & _msg)
    {
#	if defined(NB_WINDOWS) && defined(_MSC_VER)
        OutputDebugString(_msg.c_str());
#	endif

		stringa a(_msg);
		fputs(a.c_str(), stdout);
		fflush(stdout);
    }
#endif // NB_DEBUG

#if defined(NB_CFG_MEMCHECK) || defined(NB_DEBUG)
    void basic_debug_write_line(const string & _msg)
    {
#	if defined(NB_WINDOWS) && defined(_MSC_VER)
        OutputDebugString(_msg.c_str());
        OutputDebugString(L"\n");
#	endif

		stringa a(_msg);
		fputs(a.c_str(), stdout);
		fputs("\n", stdout);
		fflush(stdout);
    }
#endif

}

#undef malloc
#undef realloc
#undef calloc
#undef free
namespace e
{
	void * nd_malloc(size_t _sz)
	{ return malloc(_sz); }
	void * nd_realloc(void * _p, size_t _sz)
	{ return realloc(_p, _sz); }
	void * nd_calloc(size_t _num, size_t _sz)
	{ return calloc(_num, _sz); }
	void   nd_free(void * _p)
	{ free(_p); }
}

#ifdef NB_CFG_MEMCHECK

void * operator new(size_t _sz) throw (std::bad_alloc)
{ return e::debug_malloc(_sz, 0, 0, e::nbDmbtNew); }

void * operator new[](size_t _sz) throw (std::bad_alloc)
{ return e::debug_malloc(_sz, 0, 0, e::nbDmbtNewArray); }

void operator delete(void * _p) throw (std::bad_alloc)
{ e::debug_free(_p, e::nbDmbtNew); }

void operator delete[](void * _p) throw (std::bad_alloc)
{ e::debug_free(_p, e::nbDmbtNewArray); }

void * operator new(size_t _sz, const char * _file, int _line)
{ return e::debug_malloc(_sz, _file, _line, e::nbDmbtNew); }

void operator delete(void * _p, const char *, int)
{ e::debug_free(_p, e::nbDmbtNew);}

void * operator new[](size_t _sz, const char * _file, int _line)
{ return e::debug_malloc(_sz, _file, _line, e::nbDmbtNewArray); }

void operator delete[](void * _p, const char *, int)
{ e::debug_free(_p, e::nbDmbtNewArray); }

namespace e
{
	static void debug_memory_report_error(const string & _msg, const string & _file, int _line)
    { basic_debug_write_line(_file + L"(" + string(_line) + L"): " + _msg); }

	static string g_get_dmbt_allocted_by(NB_DEBUG_MEMORY_BLOCK_TYPE _type)
	{
		switch(_type)
		{
		case nbDmbtC:
			return L"malloc,calloc or realloc";
		case nbDmbtNew:
			return L"\"operator new\"";
		case nbDmbtNewArray:
			return L"\"opearate new[]\"";
		default:
			E_BASIC_ASSERT(0);
			return L"unkown function";
		}
	}

	static string g_get_dmbt_delete_by(NB_DEBUG_MEMORY_BLOCK_TYPE _type)
	{
		switch(_type)
		{
		case nbDmbtC:
			return L"free";
		case nbDmbtNew:
			return L"\"delete\"";
		case nbDmbtNewArray:
			return L"\"delete[]\"";
		default:
			E_BASIC_ASSERT(0);
			return L"\"unkown_release\"";
		}
	}
#define E_DEBUG_MEMORY_COOKIE 0xA5A5A5A5
	// +----------+---------+===========================+--------+
	//   header     cookie0     allocated  block          cookie1
	struct NB_DEBUG_MEMORY_HEADER
	{
		void dump(bool _extraInfo);
		int order;
		const char * file;
		int line;
		NB_DEBUG_MEMORY_BLOCK_TYPE type;
		size_t size;
		uint32 cookie0;
	};

	typedef NB_DEBUG_MEMORY_HEADER * DMHPtr;

	class DMHSet
	{
	public:
		struct Node
		{
			Node * parent;
			Node * left;
			Node * right;
			int priority;
			DMHPtr data;
		};

		DMHSet() : root(0)
		{}

		~DMHSet()
		{ clear(); }


		Node * begin()
		{
			Node * p = root;
			if(p)
			{
				while(p->left)
				{
					p = p->left;
				}
			}
			return p;
		}

		bool empty() const
		{ return root == 0; }

		Node * next(Node * _p)
		{
			Node * p = _p;
			E_ASSERT(p);
			if(p->right)
			{
				p = p->right;
				while(p->left)
				{
					p = p->left;
				}
			}
			else if(p->parent)
			{
				Node * p1;
				do
				{
					p1 = p;
					p = p->parent;
				}while(p && p->right == p1);
			}
			else
			{
				p = 0;
			}
			return p;
		}

		void insert(const DMHPtr & _d)
		{
			Node * n = (Node *) malloc(sizeof(Node));
			memset(n, 0, sizeof(Node));
			n->priority  = random();
			n->data = _d;

			if(root == 0)
			{
				root = n;
				return;
			}

			Node * p = root;
			while(true)
			{
				if(n->data < p->data)
				{
					if(p->left)
					{
						p = p->left;
					}
					else
					{
						n->parent = p;
						p->left = n;
						break;
					}
				}
				else
				{
					if(p->right)
					{
						p = p->right;
					}
					else
					{
						n->parent = p;
						p->right = n;
						break;
					}
				}
			}
			adjust(n);
		}

		void clear()
		{
			for(Node * it = begin(); it != 0; it = erase(it))
			{}
		}

		Node * erase(Node * _p)
		{
			Node * p = _p;
			while(p->left && p->right)
			{
				Node * pp = p->parent;
				Node * p1 = p->left;
				Node * p2 = p->right;
				Node * &pm = pp ? (pp->left == p ? pp->left : pp->right) : root;
				if(p->left->priority < p->right->priority)
				{
					pm = p2;
					p2->parent = pp;

					p->right = p2->left;
					if(p->right)
					{
						p->right->parent = p;
					}

					p2->left = p;
					p->parent = p2;
				}
				else
				{
					pm = p1;
					p1->parent = pp;

					p->left = p1->right;
					if(p->left)
					{
						p->left->parent = p;
					}

					p1->right = p;
					p->parent = p1;

				}
			}

			{
				Node * pp = p->parent;
				Node * &pm = pp ? (pp->left == p ? pp->left : pp->right) : root;
				if(p->right)
				{
					pm = p->right;
					p->right->parent = pp;
					free(p);
					return pm;
				}
				else
				{
					pm = p->left;
					if(p->left)
					{
						p->left->parent = pp;
					}
					free(p);
					return pp;
				}
			}
		}

		void erase(const DMHPtr & _d)
		{
			Node * it = find(_d);
			if(it != 0)
			{
				erase(it);
			}
		}

		Node * find(const DMHPtr & _d)
		{
			Node * p = root;
			while(p)
			{
				if(_d == p->data)
				{
					return p;
				}

				if(_d < p->data)
				{
					p = p->left;
				}
				else
				{
					p = p->right;
				}
			}
			return 0;
		}

	private:
		Node * root;
		static int random()
		{
			static unsigned int randomSeed = 1;
			randomSeed = (unsigned int)(randomSeed * 1103515245 + 12345) % 32768;
			return randomSeed;
		}
		void adjust(Node * _p)
		{
			Node * p = _p;
			while(p->parent)
			{
				if(p->parent->priority  < p->priority )
				{
					if(p->parent->left == p)
					{
						p->parent->left = p->right;
						if(p->right)
						{
							p->right->parent = p->parent;
						}
						p->right = p->parent;
						p->parent = p->right->parent;
						p->right->parent = p;
						if(p->parent)
						{
							if(p->parent->left == p->right)
							{
								p->parent->left = p;
							}
							else
							{
								p->parent->right = p;
							}
						}
						else
						{
							root = p;
							break;
						}
					}
					else
					{
						p->parent->right = p->left;
						if(p->left)
						{
							p->left->parent = p->parent;
						}
						p->left = p->parent;
						p->parent = p->left->parent;
						p->left->parent = p;
						if(p->parent)
						{
							if(p->parent->right == p->left)
							{
								p->parent->right = p;
							}
							else
							{
								p->parent->left = p;
							}
						}
						else
						{
							root = p;
							break;
						}
					}
				}
				else
				{
					p = p->parent;
				}
			}
		}
	};

	static Mutex & g_debug_memory_mutex1()
	{
		static Mutex _debug_memory_mutex;
		return _debug_memory_mutex;
	}

	static int  g_debug_memory_break_at_alloc = -1;
	static int g_debug_memory_block_order = 0;
	static DMHSet * g_debug_memory_heap = 0;
	static void nb_debug_memory_init();
	static void nb_debug_memory_close();
	static void g_debug_memory_heap_insert(NB_DEBUG_MEMORY_HEADER * _p)
	{
		ScopeLock sl(g_debug_memory_mutex1());

		g_debug_memory_block_order++;
		_p->order = g_debug_memory_block_order;
		g_debug_memory_heap->insert(_p);
		if(_p->order == g_debug_memory_break_at_alloc)
		{
			E_DEBUG_BREAK;
		}
	}

#define	E_DEBUG_MEMORY_CHECK_COOKIE0(_h) E_BASIC_ASSERT((_h)->cookie0 == E_DEBUG_MEMORY_COOKIE)
#define	E_DEBUG_MEMORY_CHECK_COOKIE1(_h) E_BASIC_ASSERT(*((uint32*)((char*)(_h) + sizeof(NB_DEBUG_MEMORY_HEADER) + (_h)->size)) == E_DEBUG_MEMORY_COOKIE)

	static NB_DEBUG_MEMORY_HEADER * g_debug_memory_heap_checkout(void * _p)
	{
		ScopeLock sl(g_debug_memory_mutex1());
		//test();

		if(_p == 0)
		{
			return 0;
		}

#ifdef _MSC_VER
		E_BASIC_ASSERT(_p != (void*) 0xcdcdcdcd); 
		E_BASIC_ASSERT(_p != (void*) 0xfeeefeee); 
#endif

		NB_DEBUG_MEMORY_HEADER * h = (NB_DEBUG_MEMORY_HEADER *)(((char*)_p) - sizeof(NB_DEBUG_MEMORY_HEADER));

		E_DEBUG_MEMORY_CHECK_COOKIE0(h); // underflow?
		E_DEBUG_MEMORY_CHECK_COOKIE1(h); // overflow?

		DMHSet::Node * p;
		if(g_debug_memory_heap && (p = g_debug_memory_heap->find(h)) != 0)
		{
			g_debug_memory_heap->erase(p);
			return h;
		}
		else
		{
			E_BASIC_ASSERT(0);
			return 0;
		}
	}

#define get_debug_memory_body(_p) ((((char*)(_p)) + sizeof(NB_DEBUG_MEMORY_HEADER)))

	void * debug_malloc(size_t _sz, const char * _file, int _line, NB_DEBUG_MEMORY_BLOCK_TYPE _type)
	{
		if(g_debug_memory_heap == 0)
		{
			nb_debug_memory_init();
		}
		size_t actualSize = _sz == 0 ? 1 : _sz;
		size_t totalSize = actualSize + sizeof(NB_DEBUG_MEMORY_HEADER) + sizeof(uint32);
		char * p = (char *)malloc(totalSize);
		NB_DEBUG_MEMORY_HEADER * header = (NB_DEBUG_MEMORY_HEADER *)p;
		header->order   = 0;
		header->file    = _file;
		header->line    = _line;
		header->size    = _sz;
		header->type    = _type;
		header->cookie0 = E_DEBUG_MEMORY_COOKIE;
		*((uint32*)(p + sizeof(NB_DEBUG_MEMORY_HEADER) + header->size)) = E_DEBUG_MEMORY_COOKIE;
		g_debug_memory_heap_insert(header);

		return  p + sizeof(NB_DEBUG_MEMORY_HEADER);
	}

	void debug_chage_loc(void * _p, const char * _file, int _line)
	{
		E_ASSERT(g_debug_memory_heap != 0);
		E_ASSERT(_p != 0);
		NB_DEBUG_MEMORY_HEADER * header = g_debug_memory_heap_checkout(_p);
		if(header == 0)
		{
			E_ASSERT(0);
		}
		else
		{
			header->file    = _file;
			header->line    = _line;
			g_debug_memory_heap_insert(header);
		}
	}

	void * debug_realloc(void * _p, size_t _sz, const char * _file, int _line)
	{
		if(g_debug_memory_heap == 0)
		{
			nb_debug_memory_init();
		}

		if(_p == 0)
		{
			return debug_malloc(_sz, _file, _line, nbDmbtC);
		}

		NB_DEBUG_MEMORY_HEADER * header = g_debug_memory_heap_checkout(_p);
		if(header == 0)
		{
			debug_memory_report_error(L"[nb] debug_realloc(): re-alloc a block which was not allocated by debug memory functions.", __FILE__, __LINE__);
			E_DEBUG_BREAK;
			return realloc(_p, _sz);
		}

		if(header->type != nbDmbtC)
		{
			header->dump(true);
			debug_memory_report_error(L"[nb] debug_realloc(): re-alloc a block which was allocated by " + g_get_dmbt_allocted_by(header->type) , __FILE__, __LINE__);
			E_DEBUG_BREAK;
		}

		if(_sz == 0)
		{
			return debug_malloc(_sz, _file, _line, nbDmbtC);
		}

		void * ret = debug_malloc(_sz, _file, _line, nbDmbtC);
		_sz = header->size < _sz ? header->size : _sz;
		memcpy(ret, _p, _sz);
		free(header);
		return ret;
	}

	void * debug_calloc(size_t _num, size_t _sz1, const char * _file, int _line)
	{
		void * p = debug_malloc(_sz1 * _num, _file, _line, nbDmbtC);
		memset(p, 0, _sz1 * _num);
		return p;
	}

	void debug_free(void * _p, NB_DEBUG_MEMORY_BLOCK_TYPE _type)
	{
		if(_type != nbDmbtC && _type != nbDmbtNew && _type != nbDmbtNewArray)
		{
			debug_memory_report_error(L"[nb] debug_free(): Invalid param : _type = " + string((int)_type),  __FILE__, __LINE__);
			E_DEBUG_BREAK;
		}

		if(_p == 0)
		{
			return;
		}

		if(g_debug_memory_heap == 0)
		{
			nb_debug_memory_init();
		}

		NB_DEBUG_MEMORY_HEADER * header = g_debug_memory_heap_checkout(_p);
		if(header == 0)
		{
			debug_memory_report_error(L"[nb] debug_free(): " + g_get_dmbt_delete_by(_type) + L" a block which was NOT allocated by debug memory functions. or it has been free (or realloc) before (maybe in anthor thread).", __FILE__, __LINE__);
			E_DEBUG_BREAK;
			return;
		}

		if(header->type != _type)
		{
			debug_memory_report_error(L"[nb] debug_realloc(): " + g_get_dmbt_delete_by(_type) + L" a block which was allocated by " + g_get_dmbt_allocted_by(header->type), __FILE__, __LINE__);
			header->dump(true);
			E_DEBUG_BREAK;
		}

		free(header);
	}

	static void Basicdebug_write(const string & _msg)
    {
#if defined(NB_WINDOWS) && defined(_MSC_VER)
        OutputDebugString(_msg.c_str());
#endif

#if defined(NB_LINUX) || defined(__GNUC__)
		stringa a(_msg);
		fputs(a.c_str(), stdout);
		fflush(stdout);
#endif
    }

	void set_debug_memory_break_at_alloc(int _order)
	{
		ScopeLock sl(g_debug_memory_mutex1());
		g_debug_memory_break_at_alloc = _order;
	}
	void NB_DEBUG_MEMORY_HEADER::dump(bool _extraInfo)
	{
		if(_extraInfo)
		{
			basic_debug_write_line(L"[nb] Details of this memory block:");
			basic_debug_write_line(L"*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
		}
		string t, t1;
		size_t sz1 = 32 < size ? 32 : size;
		char * body = (char*)get_debug_memory_body(this);
		t = PointerToHex(body);
		t1.clear();
		for(size_t i=0; i<sz1; i++)
		{
			if(body[i]>=32)
			{
				t1.append((wchar_t)body[i]);
			}
			else
			{
				t1.append(L".");
			}
		}

//		for(uint n=0; n<sz1; n++)
//		{
//			t2+= ByteToHex(body[n]);
//		}


		if(file == (const char*)1)
		{
			Basicdebug_write(L"  same loc: ");
		}
		else if(file)
		{
			Basicdebug_write(string(file) + L"(" +  string(line) +  L"): ");
		}
		else
		{
			Basicdebug_write(L"unkown loc: ");
		}

		basic_debug_write_line(L"[" + string(order) + L"] " + t + L" (" + string((int)size) + L" bytes)  \"" +  t1 + L"\"");
	//	basic_debug_write_line(L"  " + t2);
		if(_extraInfo)
		{
			basic_debug_write_line(L"*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
		}
	}

	void nb_debug_memory_init()
	{
		// // NB_PROFILE_INCLUDE;
		ScopeLock sl(g_debug_memory_mutex1());
		if(g_debug_memory_heap)
		{
			return;
		}
		static bool _debug_memory_ever_inited = false;
		E_ASSERT(!_debug_memory_ever_inited); // 有全局对象在构造时分配内存?
		_debug_memory_ever_inited = true;
		g_debug_memory_heap = (DMHSet*)malloc(sizeof(DMHSet));
		new(g_debug_memory_heap) DMHSet();

		basic_debug_write_line(L"[nb] Debug memory pool initialized.");
		atexit(&nb_debug_memory_close);
	}

	static int g_debug_memory_compare_loc(const void *_l, const void *_r)
	{
		NB_DEBUG_MEMORY_HEADER* l = *((NB_DEBUG_MEMORY_HEADER**)_l);
		NB_DEBUG_MEMORY_HEADER* r = *((NB_DEBUG_MEMORY_HEADER**)_r);
		return l->file == r->file ? (l->line - r->line) :  (l->file - r->file);
	}

	/*
	static int g_debug_memory_compare_order(const void *_l, const void *_r)
	{
		NB_DEBUG_MEMORY_HEADER* l = *((NB_DEBUG_MEMORY_HEADER**)_l);
		NB_DEBUG_MEMORY_HEADER* r = *((NB_DEBUG_MEMORY_HEADER**)_r);
		return l->order - r->order;
	}
	*/

	void nb_debug_memory_close()
	{
		// // NB_PROFILE_INCLUDE;
		E_BASIC_ASSERT(g_debug_memory_heap != 0);
		if(g_debug_memory_heap->empty())
		{
			basic_debug_write_line(L"[nb] No memory leaks.");
		}
		else
		{
			// collect data
			int leaks_count = 0;
			for(DMHSet::Node * it = g_debug_memory_heap->begin(); it != 0; it = g_debug_memory_heap->next(it))
			{
				leaks_count++;
			}

			NB_DEBUG_MEMORY_HEADER** leaks = (NB_DEBUG_MEMORY_HEADER**) malloc(leaks_count * sizeof(NB_DEBUG_MEMORY_HEADER*));
			//memset(leaks, 0, leaks_count * sizeof(NB_DEBUG_MEMORY_HEADER*));
			int i=0;
			for(DMHSet::Node * it = g_debug_memory_heap->begin(); it != 0; it = g_debug_memory_heap->next(it))
			{
				leaks[i++] = it->data;
			}

			qsort(leaks, leaks_count, sizeof(NB_DEBUG_MEMORY_HEADER*), &g_debug_memory_compare_loc);
			NB_DEBUG_MEMORY_HEADER* last = 0;
			for(i=0; i<leaks_count; i++)
			{
				NB_DEBUG_MEMORY_HEADER* p =leaks[i];
				if(last && p->file == last->file && p->line == last->line)
				{
					p->file = (const char*)1;
				}
				else
				{
					last = p;
				}
			}

			// qsort(leaks, leaks_count, sizeof(NB_DEBUG_MEMORY_HEADER*), &g_debug_memory_compare_order);

			basic_debug_write_line(L"[nb] DUMP MEMORY LEAKS:");
			for(i=0; i<leaks_count; i++)
			{
				NB_DEBUG_MEMORY_HEADER* p = leaks[i];
				p->dump(false);
				free(leaks[i]);
			}
			basic_debug_write_line(L"[nb] END OF DUMP MEMORY LEAK.");
			free(leaks);
		}

		g_debug_memory_heap->~DMHSet();
		free(g_debug_memory_heap);
		g_debug_memory_heap = 0;
	}
}
#endif //defined(NB_DEBUG) && defined(E_USE_DEBUG_MEMORY)

