
//#include "private.h"
#include <nbug/tl/stack.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef NB_WINDOWS
#	include <io.h> // _chsize_s
#	include <shlwapi.h>
//#	pragma comment(lib, "shlwapi")
#endif
#ifdef NB_LINUX
#	include <stdio.h>
#	include <string.h>
#	include <pwd.h>
#	include <dirent.h>
#	include <unistd.h>
#	include <fnmatch.h>
#	ifndef MAX_PATH
#		include <limits.h>
#		define MAX_PATH PATH_MAX
#	endif
#endif

#include <nbug/core/debug.h>
#include <nbug/core/file.h>
#include <nbug/core/time.h>
#include <nbug/core/thread.h>
#include <nbug/core/missing.h>


#define E_FILE_PACK_SUFFIX L"_"
#define E_FILE_PACK_MAGIC_CODE "\x80NBPD\x0D\x0A\x1A\x0A"
namespace e
{
	static Event g_file_pack_manager_func_quit;
	static Mutex g_file_pack_manager_mutex;
	static Thread * g_file_pack_manager_thread = 0;

	//FS::~FS()
	//{}

	File::~File()
	{}

	Directory::~Directory()
	{}

	//class SystemFS : public FS
	//{
	//public:
	//	FileRef OpenFile(const Path & _path, bool _write) override;
	//	DirectoryRef OpenDir(const Path & _path) override;
	//	bool IsFolder(const Path & _path) override;
	//	bool IsFile(const Path & _path) override;
	//	bool CreateFolder(const Path & _path) override;
	//	bool Copy(const Path & _src, const Path & _dst) override;
	//	bool Delete(const Path & _path) override;
	//	Path GetCurrentFolder() override;
	//	bool SetCurrentFolder(const Path & _path) override;
	//};

	//FSRef GetSystemFS()
	//{
	//	static FSRef _fs(new(SystemFS));
	//	return _fs;
	//}

	bool File::Flush()
	{
		// do nothing
		return true;
	}

	Charset File::ReadBom()
	{
		// NB_PROFILE_INCLUDE;
		uint64 pos = Tell();
		E_ASSERT(pos == 0);
		if(pos != 0)
		{
			message(L"[nb] (WW) CrtFile::ReadBom() file position foce to begining.");
			Seek(0);
		}

		/*
		Bytes Encoding Form
00 00 FE FF UTF-32, big-endian
FF FE 00 00 UTF-32, little-endian
FE FF UTF-16, big-endian
FF FE UTF-16, little-endian
EF BB BF UTF-8
		*/
		uint64 fileSize = GetSize();
		if(fileSize >= 2)
		{
			unsigned char ch0,ch1,ch2,ch3;
			Read(&ch0, 1);
			Read(&ch1, 1);
			if(ch0 == 0 && ch1 == 0 && fileSize >= 4)
			{
				Read(&ch2, 1);
				Read(&ch3, 1);
				if(ch2 == 0xfe && ch3 == 0xff)
				{
					return CHARSET_UTF32BE;
				}
			}
			else if(ch0 == 0xff && ch1 == 0xfe)
			{
				if(fileSize >= 4)
				{
					Read(&ch2, 1);
					Read(&ch3, 1);
					if(ch2 == 0 && ch3 == 0)
					{
						return CHARSET_UTF32LE;
					}
				}
				Seek(2);
				return CHARSET_UTF16LE;
			}
			else if(ch0 == 0xfe && ch1 == 0xff)
			{
				return CHARSET_UTF16BE;
			}
			else if(ch0 == 0xef && ch1 == 0xbb && fileSize >= 3)
			{
				Read(&ch2, 1);
				if(ch2 == 0xbf)
				{
					return CHARSET_UTF8;
				}
			}
		}
		Seek(0);
		return CHARSET_LOCALE;
	}

	bool File::WriteBom(const Charset & _charset)
	{
		// NB_PROFILE_INCLUDE;
		uint64 pos = Tell();
		E_ASSERT(pos == 0);
		if(pos != 0)
		{
			message(L"[nb] (WW) CrtFile::WriteBom() file position foce to begining.");
			if(!Seek(0))
			{
				return false;
			}
		}

		switch(_charset)
		{
		case CHARSET_UTF16LE:
			return Write("\xFF\xFE", 2);
		case CHARSET_UTF16BE:
			return Write("\xFE\xFF", 2);
		case CHARSET_UTF32LE:
			return Write("\xFF\xFE\x00\x00", 4);
		case CHARSET_UTF32BE:
			return Write("\x00\x00\xFE\xFF", 4);
		case CHARSET_UTF8:
			return Write("\xEF\xBB\xBF", 3);
		default:
			return true;
		}
	}

	bool File::Skip(size_t _size)
	{
		// NB_PROFILE_INCLUDE;
		return Seek(_size, SEEK_CUR);
	}

	bool File::WriteLine(const stringa & _t)
	{
		// NB_PROFILE_INCLUDE;
		if(!Write(_t))
		{
			return false;
		}

#ifdef NB_WINDOWS
		if(!Write("\r\n", 2))
		{
			return false;
		}
#else
		if(!Write("\n", 1))
		{
			return false;
		}
#endif

		return true;
	}

/*
	MemoryBlock File::ReadAll()
	{
		MemoryBlock ret ={0, 0};
		uint64 len = GetSize();
		if(len <= 0xffffffff)
		{
			ret.len = uint32(len);
			uint32 len1 = ret.len + 1;
			ret.data = enew uint8[len1];
			ret.data[len] = 0;
			if(!Read(ret.data, (size_t)(len)))
			{
				delete[] ret.data;
				ret.data = 0;
				ret.len = 0;
			}
		}
		return ret;
	}
*/

#if !defined(_MSC_VER)
#	if defined(UNICODE) && defined(NB_WINDOWS)
	int _wfopen_s(FILE ** _pFile, const wchar_t * _filename, const wchar_t * _mode)
	{
		*_pFile = _wfopen(_filename, _mode);
		return *_pFile == NULL ? 1 : 0;
	}
#	else
	int fopen_s(FILE ** _pFile, const char * _filename, const char * _mode)
	{
		*_pFile = fopen(_filename, _mode);
		return *_pFile == NULL ? 1 : 0;
	}
#	endif
#endif

	static FileRef _OpenRealFile(const Path & _path, bool _write);
	class CrtFile : public File
	{
	private:
		FILE * file;
		bool managed;
		CrtFile();
		friend FileRef _OpenRealFile(const Path & _path, bool _write);
		void Close();
		virtual ~CrtFile();
	public:
		bool RandomAccess() const override;
		uint64 GetSize() const override;
		bool SetSize(uint64 _n) override;
		bool Read(void * _buf, size_t _size) override;;
		bool ReadLine(stringa & _t) override;
		size_t ReadSome(void * _buf, size_t _size) override;;
		bool Write(const void * _buf, size_t _size) override;;
		bool Unread(unsigned char _byte) override;;
		bool Write(const stringa & _t) override;
		bool Eof() const override;
		bool Flush() override;
		int64 Tell() const override;
		bool Seek(int64 _pos,int _type = SEEK_SET) override;
		void Attach(FILE * _file, bool _managed); // note: use _open_osfhandle and _fdopen to convert from HFILE.
		void * Detach();
	};

#define FILE_PTR ((FILE*)file)
	CrtFile::CrtFile()
	{
		file = 0;
		managed = true;
	}

	bool CrtFile::RandomAccess() const
	{
		return true;
	}

	void CrtFile::Attach(FILE * _p, bool _managed)
	{
		// NB_PROFILE_INCLUDE;

		if(FILE_PTR)
		{
			Close();
		}
		file = _p;
		managed = _managed;
	}

	void CrtFile::Close()
	{
		// NB_PROFILE_INCLUDE;
		if(FILE_PTR)
		{
			if(managed)
			{
				::fclose(FILE_PTR);
			}
			file = 0;
		}
		managed = true;
	}

	CrtFile::~CrtFile()
	{
		// NB_PROFILE_INCLUDE;
		Close();
	}

	bool CrtFile::Read(void * _buf, size_t _size)
	{
		// NB_PROFILE_INCLUDE;
		E_ASSERT(::ferror(FILE_PTR) == 0);
		E_ASSERT(::feof(FILE_PTR) == 0);
		return ::fread(_buf, 1, _size, FILE_PTR) == _size;
	}

	size_t CrtFile::ReadSome(void * _buf, size_t _size)
	{
		// NB_PROFILE_INCLUDE;
		return ::fread(_buf, 1, _size, FILE_PTR);
	}


	bool CrtFile::Unread(unsigned char _byte)
	{
		// NB_PROFILE_INCLUDE;
		return EOF != ::ungetc(_byte, FILE_PTR);
	}

	bool CrtFile::Write(const void * _buf, size_t _size)
	{
		// NB_PROFILE_INCLUDE;
		return ::fwrite(_buf, 1, _size, FILE_PTR) == _size;
	}

	bool CrtFile::Write(const stringa & _t)
	{
		// NB_PROFILE_INCLUDE;
		size_t n = _t.length();
		return ::fwrite(_t.c_str(), 1, n, FILE_PTR) == n;
	}

//	void CrtFile::WriteLine(const stringw & _t)
//	{
//		Write(_t);
//#if defined(NB_WINDOWS)
//		Write(L"\r\n", 4);
//#else
//		Write(L"\n", 2);
//#endif
//	}

	uint64 CrtFile::GetSize() const
	{
		if(FILE_PTR)
		{
#ifdef NB_WINDOWS
			struct _stat buf;
			int rc = _fstat(_fileno(FILE_PTR), &buf);
#else
			struct stat buf;
			int rc = fstat(fileno(FILE_PTR), &buf);
#endif
			if(rc == 0)
			{
				return buf.st_size;
			}
			else
			{
				long pos = ftell(FILE_PTR);
				long size;
				::fseek(FILE_PTR, 0, SEEK_END) ;
				size = ::ftell(FILE_PTR) ;
				::fseek(FILE_PTR, pos, SEEK_SET);
				return size;
			}
		}
		else
		{
			return 0;
		}
	}

	bool CrtFile::Eof() const
	{
		// NB_PROFILE_INCLUDE;
		char tmp;
		int rc = fread(&tmp, 1, 1, FILE_PTR);
		bool ret = feof(FILE_PTR) ? true : false;
		ungetc(tmp, FILE_PTR);
		return ret;
	}


	int64 CrtFile::Tell() const
	{
		// NB_PROFILE_INCLUDE;
#ifdef _MSC_VER
		return _ftelli64(FILE_PTR);
#elif defined(NB_WINDOWS)
        return ftell(FILE_PTR);
#else
		return ftello64(FILE_PTR);
#endif
	}

	bool CrtFile::Seek(int64 _n, int _type)
	{
		// NB_PROFILE_INCLUDE;
#ifdef _MSC_VER
		return ::_fseeki64(FILE_PTR, _n, _type) == 0;
#elif defined(NB_WINDOWS)
        return ::fseek(FILE_PTR, _n, _type) == 0;
#else
		return ::fseeko64(FILE_PTR, _n, _type) == 0;
#endif
	}


	bool CrtFile::Flush()
	{
		// NB_PROFILE_INCLUDE;
		return fflush(FILE_PTR) == 0;
	}

	bool CrtFile::ReadLine(stringa & _t)
	{
		// NB_PROFILE_INCLUDE;
		_t.clear();
		_t.reserve(4000); // TODO: release extra memory
		return fgets(_t.c_str(), 4000, FILE_PTR) != 0;
	}

	bool CrtFile::SetSize(uint64 _n)
	{
		// NB_PROFILE_INCLUDE;
		rewind(FILE_PTR);
#	ifdef NB_WINDOWS
		int fd = _fileno(FILE_PTR);
		//return 0 == _chsize_s(fd, _n); // io.h
		return 0 ==  _chsize(fd, (long)_n); // io.h
#	else
		int fd = fileno(FILE_PTR);
		return 0 == ftruncate(fd, _n);
#	endif
	}

	void * CrtFile::Detach()
	{
		// NB_PROFILE_INCLUDE;
		void * ret = file;
		file = 0;
		return ret;
	}

#ifdef NB_WINDOWS
	void GetAllDriver(StringArray & _ret)
	{
		// NB_PROFILE_INCLUDE;
		_ret.clear();
		Char buf[1024];
		DWORD n = ::GetLogicalDriveStrings(1024, buf);
		if(n == 0)
		{
			return;
		}
		Char * p = buf;
		Char * end = buf + n;

		while(p < end)
		{
			_ret.push_back(string(p));
#	ifdef UNICODE
			p+= wcslen(p)+1;
#	else
			p+= strlen(p)+1;
#	endif
		}
	}
#endif

	static DirectoryRef _OpenRealDir(const Path & _path);
	class SystemDirectory : public Directory
	{
		struct DIRSTACK_ITEM
		{
	#ifdef NB_WINDOWS
			WIN32_FIND_DATA findData;
			HANDLE hFind;
	#endif
	#ifdef NB_LINUX
			DIR * dir;
			dirent * ent;
	#endif
		};

		typedef e::Stack<DIRSTACK_ITEM> DIRSTACK;
		DIRSTACK dirStack;

		friend DirectoryRef _OpenRealDir(const Path & _path);
		Path rootFolder;
		Path currentRelativeFolder;
		inline Path GetCurrentFolder() const
		{ return rootFolder | currentRelativeFolder; }
		DirectoryRef correspond;
	public:
//		FSRef GetFS() const override;
		bool MoveNext(bool _enterSubDir) override;
		string GetName() const  override;
		bool IsEnd() const override;
		bool IsInFilePack() const override;
		bool IsDir() const override;
		bool IsHidden() const override;
		Path GetRelativePath() const override;
		const Path & GetRootPath() const override;
		~SystemDirectory();
	};

	//FSRef SystemDirectory::GetFS() const
	//{
	//	return GetSystemFS();
	//}

#ifdef NB_WINDOWS
	inline static bool IsDotOrDotDot(TCHAR * cFileName)
	{
		return cFileName[0] == L'.' && (cFileName[1] == 0 || (cFileName[1] == L'.' && cFileName[2] == 0));
	}
#endif

#ifdef NB_LINUX
	inline static bool IsDotOrDotDot(char * cFileName)
	{
		return cFileName[0] == L'.' && (cFileName[1] == 0 || (cFileName[1] == L'.' && cFileName[2] == 0));
	}
#endif

	bool SystemDirectory::MoveNext(bool _enterSubDir)
	{
		if(dirStack.empty())
		{
			return correspond ? correspond->MoveNext(_enterSubDir) : false;
		}

#ifdef NB_WINDOWS
		DIRSTACK_ITEM * top = &dirStack.top();
		E_ASSERT(top->hFind != 0);

		if(_enterSubDir && (top->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			Path newRelativePath = currentRelativeFolder | top->findData.cFileName;
			Path newFullPath = rootFolder | newRelativePath;

			DIRSTACK_ITEM sub;
			sub.hFind = FindFirstFile((newFullPath.GetString() + "/*").c_str(), &sub.findData);
			if(sub.hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					if(!IsDotOrDotDot(sub.findData.cFileName))
					{
						// enter sub folder
						dirStack.push(sub);
						currentRelativeFolder = newRelativePath;
						return true;
					}
				}while(FindNextFile(sub.hFind, &sub.findData));
				// no file in folder
				FindClose(sub.hFind);
			}
		}

		do
		{
			top = &dirStack.top();
			while(FindNextFile(top->hFind, &top->findData))
			{
				if(!IsDotOrDotDot(top->findData.cFileName))
				{
					// find next file
					return true;
				}
			}
			// no file remain in this folder, goto parent
			::FindClose(top->hFind);
			dirStack.pop();
			currentRelativeFolder.UpOneLevel();
		}while(!dirStack.empty());

#endif

#ifdef NB_LINUX
		DIRSTACK_ITEM * top = &dirStack.top();
		E_ASSERT(top->dir != 0);

		if(_enterSubDir && IsDir())
		{
			Path newRelativePath = currentRelativeFolder | top->ent->d_name;
			Path newFullPath = rootFolder | newRelativePath;

			DIRSTACK_ITEM sub;
			sub.dir = ::opendir(newFullPath.GetStringA().c_str());
			if(sub.dir != 0)
			{
				while((sub.ent = ::readdir(sub.dir)) != 0)
				{
					if(!IsDotOrDotDot(sub.ent->d_name))
					{
						dirStack.push(sub);
						currentRelativeFolder = newRelativePath;
						return true;
					}
				}
				closedir(sub.dir);
			}
		}

		do
		{
			top = &dirStack.top();
			while((top->ent = ::readdir(top->dir)) != 0)
			{
				if(!IsDotOrDotDot(top->ent->d_name))
				{
					// find next file
					return true;
				}
			}
			// no file remain in this folder, goto parent
			::closedir(top->dir);
			dirStack.pop();
			currentRelativeFolder.UpOneLevel();
		}while(!dirStack.empty());

#endif
		return correspond ? true : false;
	}

	string SystemDirectory::GetName() const
	{
		if(dirStack.empty())
		{
			return correspond ? correspond->GetName() : L"";
		}

#ifdef NB_WINDOWS
		return string(dirStack.top().findData.cFileName);
#endif
#ifdef NB_LINUX
		return string(dirStack.top().ent->d_name);
#endif
	}

	Path SystemDirectory::GetRelativePath() const
	{
		if(dirStack.empty())
		{
			return correspond ? correspond->GetRelativePath() : Path();
		}


#ifdef NB_WINDOWS
		return currentRelativeFolder | string(dirStack.top().findData.cFileName);
#endif
#ifdef NB_LINUX
		return currentRelativeFolder | string(dirStack.top().ent->d_name);
#endif
	}

	const Path & SystemDirectory::GetRootPath() const
	{
		return rootFolder;
	}


	bool SystemDirectory::IsEnd() const
	{
		return dirStack.empty() 
			&& (!correspond || correspond->IsEnd());
	}

	bool SystemDirectory::IsInFilePack() const
	{
		return dirStack.empty() && correspond;
	}

	bool SystemDirectory::IsDir() const
	{
		if(dirStack.empty())
		{
			return correspond ? correspond->IsDir() : false;
		}
		//uint32 flags = 0;
		const DIRSTACK_ITEM * top = &dirStack.top();
#ifdef NB_WINDOWS
		return (top->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#endif

#ifdef NB_LINUX
		stringa path1 =  GetCurrentFolder().GetStringA() + "/" + stringa(top->ent->d_name);
		struct stat s;
		if(lstat(path1.c_str(), &s) == 0)
		{
			return S_ISDIR(s.st_mode);
		}
		return false;
#endif
	}

	bool SystemDirectory::IsHidden() const
	{
		if(dirStack.empty())
		{
			return correspond ? correspond->IsHidden() : false;
		}
	//	uint32 flags = 0;
		const DIRSTACK_ITEM * top = &dirStack.top();
#ifdef NB_WINDOWS
		return (top->findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;
#endif

#ifdef NB_LINUX
		return top->ent->d_name[0] == '.';
#endif
	}

	SystemDirectory::~SystemDirectory()
	{
#ifdef NB_WINDOWS
		while(!dirStack.empty())
		{
			::FindClose(dirStack.top().hFind);
			dirStack.pop();
		}
#endif
#ifdef NB_LINUX
		while(!dirStack.empty())
		{
			::closedir(dirStack.top().dir);
			dirStack.pop();
		}
#endif
	}

	bool MatchFileName(const string & _name, const string & _pattern)
	{
#ifdef NB_WINDOWS
		return ::PathMatchSpec(_name.c_str(), _pattern.c_str()) ? true : false;
#endif

#ifdef NB_LINUX
#	ifdef UNICODE
		stringa nameA(_name);
		stringa patternA(_pattern);
		return ::fnmatch(patternA.c_str(), nameA.c_str(), 0) == 0;
#	else
		return ::fnmatch(_pattern.c_str(), _name.c_str(), 0) == 0;
#	endif
#endif
	}

	struct FilePackNode
	{
		enum
		{
			flagIsDir = 0x01,
		};
		FilePackNode * parent;
		uint64 nodeOffset;
		Array<FilePackNode *> * children;
		uint8 flags;
		uint64 bodyOffset;
		uint64 bodySize;

		string name;
		inline bool IsDirEntry() const
		{ return (flags & flagIsDir) != 0; }
		inline bool IsFileEntry() const
		{ return (flags & flagIsDir) == 0; }

		FilePackNode()
		{
			parent = 0;
			nodeOffset = 0;
			children = 0;
			flags = 0;
			bodyOffset = 0;
			bodySize = 0;
		}

		~FilePackNode()
		{
			if(children != 0)
			{
				for(size_t i=0; i<children->size(); i++)
				{
					delete children->at(i);
				}
				delete children;
			}
		}
		bool IsRoot() const
		{ return parent == 0; }

		bool Save0(FileRef & _file); // save node tree
		bool Save1(FileRef & _file, Path & _srcPath); // save files
		bool Save2(FileRef & _file); // save offsets
		bool Load(FileRef & _file);
		void DebugDump();
		FilePackNode * FindChild(const string & _name, bool _create, bool _isDir)
		{
			if(children == 0)
			{
				if(_create)
				{
					children = enew Array<FilePackNode *>;
				}
				else
				{
					return 0;
				}
			}
			Array<FilePackNode *> & children1 = * children;
			size_t sz = children1.size();
			if(sz == 0)
			{
				if(_create)
				{
					FilePackNode * ret = enew FilePackNode();
					children->push_back(ret);
					ret->name = _name;
					ret->parent = this;
					if(_isDir)
					{
						ret->children = enew Array<FilePackNode *>;
						ret->flags|= flagIsDir;
					}
					return ret;
				}
				else
				{
					return 0;
				}
			}

			int l = 0;
			int r = sz - 1;
			while(l < r)
			{
				int m = (r + l) / 2;
				int c = _name.compare(children1[m]->name);
				if(c == 0)
				{
					return children1[m];
				}
				else if(c < 0)
				{
					r = m - 1;
				}
				else
				{
					l = m + 1;
				}
			}

			r = l;
			int c = _name.compare(children1[r]->name);
			if(c == 0)
			{
				return children1[r];
			}
			else if(c > 0)
			{
				r++;
			}

			if(_create)
			{

				//
				children1.push_back(0);
				for(int i = sz; i>= r+1; i--)
				{
					children1[i] = children1[i-1];
				}
				FilePackNode * ret = enew FilePackNode();
				children1[r] = ret;
				ret->name = _name;
				ret->parent = this;
				if(_isDir)
				{
					ret->children = enew Array<FilePackNode *>;
					ret->flags|= flagIsDir;
				}
				return ret;
			}
			else
			{
				return 0;
			}
		}
	};

	class FilePackDirectory;
	class FilePackFile;
	class FilePack
	{
	public:
		FilePack * next;
		static FilePack * filePacks;
		Path packPathWithoutSharp; 
		double lifeCicle;
		double dieTime; 
		void SetAccessTime()
		{
			mutex.Lock();
			dieTime = Time::GetTicks() + lifeCicle;
			mutex.Unlock();
		}
		bool CanDelete(double _now)
		{
			ScopeLock sl(mutex);
			return dieTime < _now && dirAccessCount == 0 && fileAccessCount == 0;
		}
		int dirAccessCount;
		int fileAccessCount;

		FileRef underFile;
		Mutex mutex;
		FilePackNode root;
		FilePack();
		~FilePack();
		FileRef OpenFile(const Path & _relativePath, bool _write = false) ;
		DirectoryRef OpenDir(const Path & _relativePath) ;
		bool IsFolder(const Path & _relativePath) ;
		bool IsFile(const Path & _relativePath) ;
		//bool CreateFolder(const Path & _relativePath) ;
		//bool Copy(const Path & _src, const Path & _dst) ;
		//bool Delete(const Path & _path) ;
		FilePackNode * FindNode(const StringArray &_path, bool _createMissing, bool _isdir);
	};

	FilePack * FilePack::filePacks = 0;

	FilePack::FilePack()
	{
		dirAccessCount = 0;
		fileAccessCount = 0;
		underFile = 0;
		root.flags |= FilePackNode::flagIsDir;
		root.name = "*";
		next = filePacks;
		filePacks = this;
		lifeCicle = 10;
		SetAccessTime();
	}

	FilePack::~FilePack()
	{
		E_TRACE_LINE(L"[nb] File pack unloaded: " + packPathWithoutSharp.GetString());
		// TODO: optimize
		FilePack * p0 = filePacks;
		FilePack * p1 = 0;
		while(p0 != this && p0 != 0)
		{
			p1 = p0;
			p0 = p0->next;
		}
		if(p0 != 0)
		{
			if(p1)
			{
				p1->next = p0->next;
			}
			else
			{
				filePacks = p0->next;
			}
		}
		else
		{
			E_ASSERT(0);
		}
	}

	FilePackNode * FilePack::FindNode(const StringArray &_path, bool _createMissing, bool _isdir)
	{
		// NB_PROFILE_INCLUDE;
		FilePackNode * p = &root;
		uint sz = _path.size();
		for(size_t i=0; i<sz; i++)
		{
			p = p->FindChild(_path[i], _createMissing, i < sz-1 || _isdir );
			if(p == 0)
			{
				break;
			}
		}
		return p;
	}

	// fix offsets
	bool FilePackNode::Save2(FileRef & _file)
	{
		// NB_PROFILE_INCLUDE;
		if(flags & flagIsDir)
		{
			if(children)
			{
				for(size_t i=0; i<children->size(); i++)
				{
					children->at(i)->Save2(_file);
				}
			}
		}
		else
		{
			// write offset into disk
			if(!_file->Seek(nodeOffset + 1, SEEK_SET)) 
			{
				E_ASSERT(0);
				return false;
			}

			if(!_file->Write(&bodyOffset, sizeof(uint64)))
			{
				E_ASSERT(0);
				return false;
			}
			if(!_file->Write(&bodySize, sizeof(uint64)))
			{
				E_ASSERT(0);
				return false;
			}
		}
		return true;
	}

	void FilePackNode::DebugDump()
	{
		StringArray path;
	//	path.push_back(name);
		FilePackNode * p = this;
		while(p->parent)
		{
			path.push_back(p->name);
			p = p->parent;
		}

		for(int i = path.size() - 1; i >= 0; i--)
		{
			E_TRACE(L" / " + path[i]);
		}
		E_TRACE_LINE("");


		if(flags & flagIsDir)
		{
			if(children)
			{
				for(size_t i=0; i<children->size(); i++)
				{
					children->at(i)->DebugDump();
				}
			}
		}
	}


	bool FilePackNode::Load(FileRef & _file)
	{
		// NB_PROFILE_INCLUDE;
		if(!_file->Read(&flags, sizeof(flags)))
		{
			E_ASSERT(0);
			return false;
		}

		if(flags & flagIsDir)
		{
			uint32 count = 0;
			if(!_file->Read(&count, sizeof(count)))
			{
				E_ASSERT(0);
				return false;
			}
			for(size_t i=0; i<count; i++)
			{
				if(children == 0)
				{
					children = enew Array<FilePackNode*>;
				}
				FilePackNode * p = enew FilePackNode();
				p->parent = this;
				children->push_back(p);
				if(!_file->Read(&p->nodeOffset, sizeof(uint64)))
				{
					E_ASSERT(0);
					return false;
				}
			}
		}
		else
		{
			if(!_file->Read(&bodyOffset, sizeof(uint64)))
			{
				E_ASSERT(0);
				return false;
			}
			if(!_file->Read(&bodySize, sizeof(uint64)))
			{
				E_ASSERT(0);
				return false;
			}
		}

		uint32 len1;
		if(!_file->Read(&len1, sizeof(len1))) // nameA.len
		{
			E_ASSERT(0);
			return false;
		}

		stringa nameA1;
		if(len1 > 0)
		{
			nameA1.reserve(len1);
			if(!_file->Read(nameA1.c_str(), len1)) // nameA
			{
				E_ASSERT(0);
				return false;
			}
			nameA1.c_str()[len1] = 0;
		}
		name = stringw(nameA1, e::CHARSET_UTF8);

		if((flags & flagIsDir)!=0 && children != 0)
		{
			for(size_t i=0; i<children->size(); i++)
			{
				FilePackNode * p = children->at(i);
				if(!_file->Seek(p->nodeOffset, SEEK_SET)
					|| !p->Load(_file))
				{
					E_ASSERT(0);
					return false;
				}
			}
		}
		return true;
	}

	bool FilePackNode::Save0(FileRef & _file)
	{
		// NB_PROFILE_INCLUDE;
		// must save sub node first, to get their offsets
		if(children != 0)
		{
			for(size_t i=0; i<children->size(); i++)
			{
				children->at(i)->Save0(_file);
			}
		}

		// get offset of current node
		this->nodeOffset = _file->Tell();

		// save current node
		if(!_file->Write(&flags, sizeof(flags)))
		{
			E_ASSERT(0);
			return false;
		}

//		static const uint64 _zero = 0;
		if(flags & flagIsDir)
		{
			// sub node count
			//E_ASSERT(children == 0 || children->size() != 0);
			uint32 count = children ? children->size() : 0;
			if(!_file->Write(&count, sizeof(count)))
			{
				E_ASSERT(0);
				return false;
			}

			// offset of each node
			for(size_t i=0; i<count; i++)
			{
				FilePackNode * child = children->at(i);
				if(!_file->Write(&child->nodeOffset, sizeof(uint64)))
				{
					E_ASSERT(0);
					return false;
				}
			}
		}
		else
		{
			// reserver room for offset and size of file
			uint64 _roomForFile[2];
			if(!_file->Write(_roomForFile, sizeof(_roomForFile)))
			{
				E_ASSERT(0);
				return false;
			}
		}

		// store file name in utf-8
		stringa nameA(name, e::CHARSET_UTF8);
		uint32 len = nameA.length();
		if(len == 0) // failed convert to UTF8
		{
			//stringa tmp;
			len = name.length();
			for(uint32 i=0; i<len; i++)
			{
				Char ch = name[i];
				if(isalpha((char)ch) || ch == '.')
				{
					nameA.append((char)ch);
				}
				else
				{
					int ch1 = ch;
					nameA.append(stringa::format("0x%X", ch1));
				}
			}
			len = nameA.length();
		}
		if(!_file->Write(&len, sizeof(len))) // nameA.len
		{
			E_ASSERT(0);
			return false;
		}

		if(len > 0 && !_file->Write(nameA.c_str(), len)) // nameA
		{
			E_ASSERT(0);
			return false;
		}

		return true;
	}

	bool FS::Copy(File * _src, File * _dst, uint64 _size)
	{
		if(_src == 0)
		{
			E_ASSERT(0);
			return false;
		}

		const int bufSize = 0x2000;
		char buf[bufSize];
		while(_size >= bufSize)
		{
			if(!_src->Read(buf, bufSize))
			{
				E_ASSERT(0);
				return false;
			}
			if(!_dst->Write(buf, bufSize))
			{
				E_ASSERT(0);
				return false;
			}
			_size-= bufSize;
		}
		if(_size)
		{
			if(!_src->Read(buf, (size_t)_size))
			{
				E_ASSERT(0);
				return false;
			}
			if(!_dst->Write(buf, (size_t)_size))
			{
				E_ASSERT(0);
				return false;
			}
		}
		//delete[] buf;
		return true;
	}

	Path FS::ParseShortcutFile(const Path & _path)
	{
		uint8 buf[6];
		stringa line;
		FileRef file = FS::OpenFile(_path);
		if(file 
			&& file->Read(buf, 6) 
			&& memcmp(buf, "NBLINK", 6) == 0
			&& file->ReadLine(line)
			&& file->ReadLine(line)
			)
		{

			line.trim();
			Path ret = string(line);
			if(ret.IsValid())
			{
				if(ret.IsRelative())
				{
					ret = _path.GetParentFolder() | ret;
				}
				return ret;
			}
			else
			{
				message(L"[nb] (WW) Bad shortcut target: " + line);
			}
		}

		return _path;		
	}


	bool FilePackNode::Save1(FileRef & _file, Path & _srcPath)
	{
		// NB_PROFILE_INCLUDE;
		if(children)
		{
			size_t count = children->size();
			// files at front
			for(size_t i = 0; i < count; i++)
			{
				FilePackNode * child = children->at(i);
				if((child->flags & flagIsDir) == 0)
				{
					if(!child->Save1(_file, _srcPath))
					{
						E_ASSERT(0);
						return false;
					}
				}
			}

			// sub folder at back
			for(size_t i = 0; i < count; i++)
			{
				FilePackNode * child = children->at(i);
				if(child->flags & flagIsDir)
				{
					E_ASSERT(!child->name.empty());
					_srcPath.Join(child->name);
					if(!child->Save1(_file, _srcPath))
					{
						E_ASSERT(0);
						_srcPath.UpOneLevel();
						return false;
					}
					_srcPath.UpOneLevel();
				}
			}
		}
		else
		{
			// current node
			FileRef f = FS::OpenFile(_srcPath | name, true);
			if(f)
			{
				bodyOffset = _file->Tell();
				bodySize = f->GetSize();
				if(!f->Seek(0, SEEK_SET))
				{
					E_ASSERT(0);
					return false;
				}
				if(!FS::Copy(f.ptr(), _file, bodySize))
				{
					E_ASSERT(0);
					return false;
				}
			}
			else
			{
				bodySize = 0;
			}

		}

		return true;
	}

	bool CreateFilePack(FileRef _dst, DirectoryRef _src, List<string> & _exclude)
	{
		// NB_PROFILE_INCLUDE;
		if(!_dst || ! _src)
		{
			E_ASSERT(0);
			return false;
		}
		if(!_dst->SetSize(0) || !_dst->Seek(0, SEEK_SET))
		{
			E_ASSERT(0);
			return false;
		}

		FilePack pack;

		bool to_add;
		do
		{
			to_add = !_src->IsHidden();
			if(to_add)
			{
				List<string>::iterator it = _exclude.begin();
				for(; it != _exclude.end(); ++it)
				{
					if(MatchFileName(_src->GetName(), *it))
					{
						to_add = false;
						break;
					}
				}
			}
			if(to_add)
			{
				pack.FindNode(_src->GetRelativePath().GetParts(), true, _src->IsDir());
			}
			else
			{
				message(L"[nb] excluded: " + _src->GetName());
			}
		}while(_src->MoveNext(to_add));

		if(pack.root.children == 0)
		{
			E_ASSERT(0);
			return false;
		}

#ifdef NB_DEBUG
		pack.root.DebugDump();
#endif

		// magic code
		const char magic[] = E_FILE_PACK_MAGIC_CODE;
		if(!_dst->Write(E_FILE_PACK_MAGIC_CODE, sizeof(magic)))// include \0
		{
			E_ASSERT(0);
			return false;
		}

		// life
		uint32 lifeTime = 60;
		if(!_dst->Write(&lifeTime, sizeof(lifeTime)))// include \0
		{
			E_ASSERT(0);
			return false;
		}


		uint64 off1 = _dst->Tell();
		// reserver a room for offset of pack.root.child
		uint64 dummy64;
		if(!_dst->Write(&dummy64, sizeof(uint64)))
		{
			E_ASSERT(0);
			return false;
		}

		//uint64 off2 = _dst->Tell();
		// node tree
		if(!pack.root.Save0(_dst))
		{
			E_ASSERT(0);
			return false;
		}
		uint64 off3 = _dst->Tell();

		// offset of pack.root
		if(!_dst->Seek(off1, SEEK_SET))
		{
			E_ASSERT(0);
			return false;
		}

		if(!_dst->Write(&pack.root.nodeOffset, sizeof(uint64)))
		{
			E_ASSERT(0);
			return false;
		}
		if(!_dst->Seek(off3, SEEK_SET))
		{
			E_ASSERT(0);
			return false;
		}

		//  file
		//FSRef fs = _src->GetFS();
		Path srcPath = _src->GetRootPath();
		if(!pack.root.Save1(_dst, srcPath))
		{
			E_ASSERT(0);
			return false;
		}

		// save offset and size for each file
		if(!pack.root.Save2(_dst))
		{
			E_ASSERT(0);
			return false;
		}

		return true;
	}


	bool Directory::IsHidden() const
	{
		return false;
	}

//	FilePack * LoadFilePack(FileRef & _src)
//	{
//		// NB_PROFILE_INCLUDE;
//		if(!_src || !_src->Seek(0, SEEK_SET))
//		{
//			E_ASSERT(0);
//			return 0;
//		}
//
//		// magic code
//		{
//			const char magic[] = E_FILE_PACK_MAGIC_CODE;
//			char buf[sizeof(magic)];
//			if(!_src->Read(buf, sizeof(magic)) || memcmp(magic, buf, sizeof(magic)) != 0 )
//			{
//				E_ASSERT(0);
//				return 0;
//			}
//		}
//
//		// root child offset
//		uint64 rootChildOffset;
//		if(!_src->Read(&rootChildOffset, sizeof(uint64)))
//		{
//			E_ASSERT(0);
//			return 0;
//		}
//
//		if(!_src->Seek(rootChildOffset, SEEK_SET))
//		{
//			E_ASSERT(0);
//			return 0;
//		}
//
//		FilePack * fp = enew FilePack();
//		//FSRef ret(fp);
//		if(!fp->root.Load(_src))
//		{
//			E_ASSERT(0);
//			delete fp;
//			return 0;
//		}
//
//		fp->underFile = _src;
//
//#ifdef NB_DEBUG
//		//fp->root.DebugDump();
//#endif
//
//		return fp;
//	}
	static int _FilePackManagerFunc(void *)
	{
		bool isEmpty = true;
		bool isQuit = false;
		while(!(isQuit = g_file_pack_manager_func_quit.IsSet()) || !isEmpty)
		{
			double nowTime = isQuit ? 1e300 : Time::GetTicks();
			g_file_pack_manager_mutex.Lock();
			FilePack * p = FilePack::filePacks;
			isEmpty = true;
			while(p)
			{
				if(p->CanDelete(nowTime))
				{
					FilePack * p1 = p;
					p = p->next;
					delete p1;
				}
				else
				{
					isEmpty = false;
					p = p->next;
				}
			}
			g_file_pack_manager_mutex.Unlock();
			Sleep(50);
		}

		return 0;
	}



	bool FilePack::IsFolder(const Path & _relativePath)
	{
		FilePackNode * p = FindNode(_relativePath.GetParts(), false, false);
		return p != 0 && p->children != 0;
	}

	bool FilePack::IsFile(const Path & _relativePath)
	{
		FilePackNode * p = FindNode(_relativePath.GetParts(), false, false);
		return p != 0 && p->children == 0;
	}

	//bool FilePack::CreateFolder(const Path & _path)
	//{
	//	E_ASSERT(0);
	//	return false;
	//}

	//bool FilePack::Copy(const Path & _src, const Path & _dst)
	//{
	//	E_ASSERT(0);
	//	return false;
	//}

	//bool FilePack::Delete(const Path & _path)
	//{
	//	E_ASSERT(0);
	//	return false;
	//}

	class FilePackDirectory : public Directory
	{
		friend class FilePack;
		FilePack * pack;
		struct DIRSTACK_ITEM
		{
			FilePackNode * dir;
			size_t entry;
		};

		typedef e::Stack<DIRSTACK_ITEM> DIRSTACK;
		DIRSTACK dirStack;

		FilePackNode * GetCurrent()
		{
			if(dirStack.empty())
			{
				return 0;
			}
			DIRSTACK_ITEM * top = &dirStack.top();
			E_ASSERT(!top->dir->children->empty());
			return top->dir->children->at(top->entry);
		}
		Path rootFolder;
		Path currentRelativeFolder;
		//inline Path GetCurrentFolder() const
		//{ return rootFolder | currentRelativeFolder; }
	public:
//		FSRef GetFS() const override;
		bool MoveNext(bool _enterSubDir) override;
		string GetName() const  override;
		bool IsEnd() const override;
		bool IsDir() const override;
		bool IsHidden() const override;
		bool IsInFilePack() const override
		{
			return true;
		}
		Path GetRelativePath() const override;
		const Path & GetRootPath() const override;
		~FilePackDirectory();
	};

	/*FSRef FilePackDirectory::GetFS() const
	{
		return fs;
	}*/

	bool FilePackDirectory::MoveNext(bool _enterSubDir)
	{
		// NB_PROFILE_INCLUDE;
		if(dirStack.empty())
		{
			return false;
		}

		DIRSTACK_ITEM * top = &dirStack.top();
		E_ASSERT(top->dir != 0);
		E_ASSERT(top->entry < top->dir->children->size());
		FilePackNode * current = top->dir->children->at(top->entry);

		// enter sub folder
		if(_enterSubDir && current->children != 0 && !current->children->empty())
		{
			DIRSTACK_ITEM sub;
			sub.dir = current;
			sub.entry = 0;
			dirStack.push(sub);
			currentRelativeFolder.Join(current->name);
			return true;
		}

		// next item
		do
		{
			DIRSTACK_ITEM * top = &dirStack.top();
			size_t sz = top->dir->children->size();
			top->entry++;
			if(top->entry < sz)
			{
				return true;
			}
			else
			{
				dirStack.pop();
				currentRelativeFolder.UpOneLevel();
			}
		}while(!dirStack.empty());

		return false;
	}

	string FilePackDirectory::GetName() const
	{
		if(dirStack.empty())
		{
			return "";
		}
		const DIRSTACK_ITEM * top = &dirStack.top();
		return top->dir->children->at(top->entry)->name;
	}

	bool FilePackDirectory::IsEnd() const
	{
		return dirStack.empty();
	}

	bool FilePackDirectory::IsDir() const
	{
		if(dirStack.empty())
		{
			return false;
		}
		const DIRSTACK_ITEM * top = &dirStack.top();
		return top->dir->children->at(top->entry)->IsDirEntry();
	}

	bool FilePackDirectory::IsHidden() const
	{
		return false; // unsupported
	}

	Path FilePackDirectory::GetRelativePath() const
	{
		if(dirStack.empty())
		{
			return Path();
		}
		const DIRSTACK_ITEM * top = &dirStack.top();
		return currentRelativeFolder | top->dir->children->at(top->entry)->name;
	}

	const Path & FilePackDirectory::GetRootPath() const
	{
		return rootFolder;
	}

	FilePackDirectory::~FilePackDirectory()
	{
		pack->mutex.Lock();
		E_ASSERT(pack->dirAccessCount > 0);
		pack->dirAccessCount--;
		pack->mutex.Unlock();
	}

	DirectoryRef FilePack::OpenDir(const Path & _path)
	{
		ScopeLock sl(g_file_pack_manager_mutex);
		ScopeLock sl1(this->mutex);
		//Path relative = _path -
		FilePackNode * node = FindNode(_path.GetParts(), false, true);
		if(node == 0 || node->children == 0 || node->children->empty())
		{
			E_ASSERT(0);
			return 0;
		}

		FilePackDirectory * ret = enew FilePackDirectory();
		ret->rootFolder = this->packPathWithoutSharp | _path;
		ret->pack = this;
		FilePackDirectory::DIRSTACK_ITEM item;
		item.dir   = node;
		item.entry = 0;
		ret->dirStack.push(item);
		ret->currentRelativeFolder = string(L".");
		dirAccessCount++;
		return ret;
	}


	class FilePackFile : public File
	{
	private:
		friend class FilePack;
		FilePack * pack;
		FileRef file;
		uint64 fileBegin;
		uint64 fileEnd;
		uint64 pos;

		FilePackFile()
		{}

		~FilePackFile()
		{
			pack->SetAccessTime();
			pack->mutex.Lock();
			E_ASSERT(pack->fileAccessCount > 0);
			pack->fileAccessCount--;
			pack->mutex.Unlock();
		}
	public:
		bool RandomAccess() const override
		{ return true; }

		uint64 GetSize() const override
		{ return fileEnd - fileBegin; }

		bool SetSize(uint64 _n) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Read(void * _buf, size_t _size) override
		{
			//uint64 pos = file->Tell();
			if(pos < fileBegin || pos + _size > fileEnd)
			{
				//E_ASSERT(0);
				return false;
			}
			{
				ScopeLock sl(pack->mutex);
				file->Seek(pos);
				if(file->Read(_buf, _size))
				{
					pos+= _size;
					return true;
				}
				else
				{
					return false;
				}
			}
		}

		bool ReadLine(stringa & _t) override
		{
			_t.clear();
			char ch;
			//DWORD  n;
			bool ret = false;
			while(Read(&ch, 1))
			{
				ret = true;
				_t.append(ch);
				if(ch == '\n')
				{
					break;
				}
			}
			return ret;
		}

		size_t ReadSome(void * _buf, size_t _size) override
		{
			//uint64 pos = file->Tell();
			if(pos < fileBegin)
			{
				E_ASSERT(0);
				return false;
			}
			if(pos + _size > fileEnd)
			{
				_size = (size_t)(fileEnd - pos);
			}
			{
				ScopeLock sl(pack->mutex);
				file->Seek(pos);
				size_t n = file->ReadSome(_buf, _size);
				pos+= n;
				return n;
			}
		}

		bool Write(const void * _buf, size_t _size) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Unread(unsigned char _byte) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Write(const stringa & _t) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Eof() const override
		{
			return pos < fileBegin || pos >= fileEnd;
		}

		bool Flush() override
		{
			E_ASSERT(0);
			return false;
		}

		int64 Tell() const override
		{
			// NOTE: stdio's file pointer may exceed EOF, but can't move before 0
			// Seek function ensure the pointer never move before 0, 
			// we has no rewind, other function can't move pointer to front.
			return pos - fileBegin;
		}

		bool Seek(int64 _pos, int _type = SEEK_SET) override
		{
			// NOTE: stdio's file pointer may exceed EOF, but can't move before 0
			// we should obey the rule
			switch(_type)
			{
			case SEEK_CUR:
				_pos+= pos;
				break;
			case SEEK_END:
				_pos+= fileEnd;
				break;
			case SEEK_SET:
				_pos+= fileBegin;
				break;
			default:
				E_ASSERT(0);
				return false;
			}

			{
				ScopeLock sl(pack->mutex);
				if((uint64)_pos >= fileBegin 
					&& file->Seek((uint64)_pos, SEEK_SET))
				{
					pos = _pos;
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	};


	FileRef FilePack::OpenFile(const Path & _path, bool _write)
	{
		ScopeLock sl(g_file_pack_manager_mutex);
		ScopeLock sl1(this->mutex);
		FilePackNode * node = this->FindNode(_path.GetParts(), false, false);
		if(node == 0 || node->children != 0)
		{
			//E_ASSERT(0); // invalid file
			return 0;
		}
		if(!underFile->Seek(node->bodyOffset))
		{
			E_ASSERT(0);
			return 0;
		}
		//message(L"[nb] Open pack file: " + _path.GetString());
		FilePackFile * f = enew FilePackFile();
		f->pack = this;
		f->file = this->underFile;
		f->fileBegin = node->bodyOffset;
		f->pos = f->fileBegin;
		f->fileEnd = node->bodyOffset + node->bodySize;
		fileAccessCount++;
		return f;
	}


	class MemoryFileAccess : public File
	{
	private:
		uint8 * buf;
		uint64 size;
		bool deleteAtExit;
		int64 pos;
		~MemoryFileAccess()
		{
			if(deleteAtExit)
			{
				delete[] buf;
			}
		}
	public:
		MemoryFileAccess(void * _buf, uint64 _size, bool _delete)
		{
			E_ASSERT((_size & 0x8000000000000000LL) == 0);
			buf = (uint8 *)_buf;
			size = _size;
			pos = 0;
			deleteAtExit = _delete;
		}

		bool RandomAccess() const override
		{
			return true;
		}

		uint64 GetSize() const override
		{
			return size;
		}

		bool SetSize(uint64 _n) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Read(void * _buf, size_t _size) override
		{
			int64 remain = size - pos;
			if(remain < (int64) _size)
			{
				return false;
			}
			uint8 * p1 = buf + pos;
			memcpy(_buf, p1, _size);
			pos+= _size;
			return true;
		}

		bool ReadLine(stringa & _t) override
		{
			E_ASSERT(0);
			return false;
		}

		size_t ReadSome(void * _buf, size_t _size) override
		{
			int64 remain = size - pos;
			if(remain < (int64) _size)
			{
				_size = (int) remain;
			}
			if(_size > 0)
			{
				uint8 * p1 = buf + pos;
				memcpy(_buf, p1, _size);
				pos+= _size;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool Write(const void * _buf, size_t _size) override
		{
			int64 remain = size - pos;
			if(remain < (int64) _size)
			{
				return false;
			}
			uint8 * p1 = buf + pos;
			memcpy(p1, _buf, _size);
			pos+= _size;
			return true;
		}

		bool Unread(unsigned char _byte) override
		{
			if(pos > 0)
			{
				pos--;
				buf[pos] = _byte;
				return true;
			}
			else
			{
				return false;
			}
		}

		bool Write(const stringa & _t) override
		{
			E_ASSERT(0);
			return false;
		}

		bool Eof() const override
		{
			return pos >= (int)size;
		}

		bool Flush() override
		{
			return true;
		}

		int64 Tell() const override
		{
			return pos;
		}

		bool Seek(int64 _pos, int _type = SEEK_SET) override
		{
			switch(_type)
			{
			case SEEK_CUR:
				pos+= _pos;
				return true;
			case SEEK_END:
				pos = size;
				return true;
			case SEEK_SET:
				pos = _pos;
				return true;
			default:
				E_ASSERT(0);
				return false;
			}
		}
	};

	FileRef CreateMemoryFile(void * _buf, size_t _size, bool _delete)
	{
		MemoryFileAccess * mf = enew MemoryFileAccess(_buf, _size, _delete);
		return mf;
	}

	FileRef CreateMemoryFile(size_t _size)
	{
		uint8 * p = enew uint8[_size];
		MemoryFileAccess * mf = enew MemoryFileAccess(p, _size, true);
		return mf;
	}


	//static void _InitFilePackManager()
	//{
	//	ScopeLock sl(g_file_pack_manager_mutex);
	//	if(g_file_pack_manager_thread == 0)
	//	{
	//		g_file_pack_manager_func_quit.Reset();
	//		g_file_pack_manager_thread = enew Thread(&_FilePackManagerFunc, 0);
	//	}
	//}

	void _CloseFilePackManager()
	{
		g_file_pack_manager_func_quit.Set();
		delete g_file_pack_manager_thread;
	}


	bool IsRealFolder(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		if(!_path.IsValid())
		{
			return false;
		}

		if(!_path.IsAbsolute())
		{
			return false;
		}

#ifdef NB_WINDOWS
		string s = _path.GetString();
		return (::PathIsRoot(s.c_str()) || ::PathIsDirectory(s.c_str())) ? true : false;
#endif

#ifdef NB_LINUX
		struct stat _t;
		if(0 == ::stat(_path.GetStringA().c_str(), &_t))
		{
			return S_ISDIR(_t.st_mode);
		}
		else
		{
			//E_ASSERT(0);
			return false;
		}
#endif
	}

	bool IsRealFile(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;

		if(!_path.IsValid())
		{
			return false;
		}

		if(!_path.IsAbsolute())
		{
			return false;
		}

#ifdef NB_WINDOWS
		string s = _path.GetString();
		return ::PathFileExists(s.c_str()) && !::PathIsDirectory(s.c_str());
#endif

#ifdef NB_LINUX
		struct stat _t = {0};
		::stat(_path.GetStringA().c_str(), &_t);
		return S_ISREG(_t.st_mode) ? true : false;
#endif
	}

	struct FilePackLoader
	{
		Path relativePath;
		FilePackLoader(const Path & _targetPath)
			: targetPath(_targetPath)
		{
			g_file_pack_manager_mutex.Lock();
		}

		~FilePackLoader()
		{
			g_file_pack_manager_mutex.Unlock();
		}

		FilePack * Load()
		{
			Path packFilePath = targetPath;
			packFilePath.AppendToLastPart(E_FILE_PACK_SUFFIX);
			bool isFile = false;
			while(true)
			{
				if(IsRealFile(packFilePath))
				{
					isFile = true;
					break;
				}
				if(packFilePath.HasParentFolder())
				{
					packFilePath.UpOneLevel();
					packFilePath.AppendToLastPart(E_FILE_PACK_SUFFIX);
				}
				else
				{
					break;
				}
			}

			if(!isFile)
			{
				return 0;
			}

			if(g_file_pack_manager_thread == 0)
			{
				g_file_pack_manager_func_quit.Reset();
				g_file_pack_manager_thread = enew Thread(&_FilePackManagerFunc, 0);
				atexit(&_CloseFilePackManager);
			}
			// now packFilePath has "#"
			FilePack * fp = _LoadFilePack(packFilePath);
			// now packFilePath has no "#"
			if(fp)
			{
				relativePath = targetPath - packFilePath;
			}
			return fp;
		}
	private:
		const Path & targetPath;
		static FilePack * _LoadFilePack(Path & _packFilePath)
		{
			ScopeLock sl(g_file_pack_manager_mutex);

			_packFilePath.RemoveLastChar();
			// 
			FilePack * p = FilePack::filePacks;
			while(p)
			{
				if(p->packPathWithoutSharp == _packFilePath)
				{
					p->SetAccessTime();
					return p;
				}
				p = p->next;
			}

			// 
	//		FSRef systemFS = GetSystemFS();
			_packFilePath.AppendToLastPart(E_FILE_PACK_SUFFIX);
			FileRef _src = _OpenRealFile(_packFilePath, false);
			if(!_src || !_src->Seek(0, SEEK_SET))
			{
				E_ASSERT(0);
				return 0;
			}

			// magic code
			{
				const char magic[] = E_FILE_PACK_MAGIC_CODE;
				char buf[sizeof(magic)];
				if(!_src->Read(buf, sizeof(magic)) || memcmp(magic, buf, sizeof(magic)) != 0 )
				{
					E_ASSERT(0);
					return 0;
				}
			}

			// life
			uint32 lifeTime = 1;
			{
				if(!_src->Read(&lifeTime, sizeof(lifeTime)))
				{
					E_ASSERT(0);
					return 0;
				}
			}

			// root child offset
			uint64 rootChildOffset;
			if(!_src->Read(&rootChildOffset, sizeof(uint64)))
			{
				E_ASSERT(0);
				return 0;
			}

			if(!_src->Seek(rootChildOffset, SEEK_SET))
			{
				E_ASSERT(0);
				return 0;
			}

			FilePack * fp = enew FilePack();
			//FSRef ret(fp);
			if(!fp->root.Load(_src))
			{
				E_ASSERT(0);
				delete fp;
				return 0;
			}

			fp->lifeCicle = (double) lifeTime;
			fp->underFile = _src;
			_packFilePath.RemoveLastChar();
			fp->packPathWithoutSharp = _packFilePath;
			fp->SetAccessTime();
	#ifdef NB_DEBUG
			//fp->root.DebugDump();
	#endif
			message(L"[nb] File pack loaded: " + fp->packPathWithoutSharp.GetString());
			return fp;
		}

	};
	
	static DirectoryRef _OpenRealDir(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		SystemDirectory * p = 0;
#ifdef NB_WINDOWS
		SystemDirectory::DIRSTACK_ITEM item;
		item.hFind = FindFirstFile((_path.GetString() + "/*").c_str(), &item.findData);
		if(item.hFind == INVALID_HANDLE_VALUE)
		{
			return 0;
		}
		do
		{
			if(!IsDotOrDotDot(item.findData.cFileName))
			{
				p = enew SystemDirectory();
				p->dirStack.push(item);
				p->rootFolder = _path;
				p->currentRelativeFolder = string(L".");
				break;
			}
		}while(FindNextFile(item.hFind, &item.findData));

		if(p == 0)
		{
			::FindClose(item.hFind);
			return 0;
		}
#endif

#ifdef NB_LINUX
		SystemDirectory::DIRSTACK_ITEM item;
		item.dir = ::opendir(_path.GetStringA().c_str());
		if(item.dir == 0)
		{
			return 0;
		}

		while((item.ent = ::readdir(item.dir)) != 0)
		{
			if(!IsDotOrDotDot(item.ent->d_name))
			{
				p = enew SystemDirectory();
				p->dirStack.push(item);
				p->rootFolder = _path;
				p->currentRelativeFolder = string(L".");
				break;
			}
		}

		if(p == 0)
		{
			closedir(item.dir);
			return 0;
		}
#endif

		FilePackLoader loader(_path);
		FilePack * fp = loader.Load();
		if(fp)
		{
			p->correspond = fp->OpenDir(loader.relativePath);
		}
		return p;
	}


	static FileRef _OpenRealFile(const Path & _path, bool _write)
	{
		// NB_PROFILE_INCLUDE;
		if(_write)
		{
			if(_path.HasParentFolder() && !FS::CreateFolder(_path.GetParentFolder()))
			{
				return 0;
			}
		}
		string textPath = _path.GetString();

		const Char * mode = 0;
		if(_write)
		{
			if(IsRealFile(_path))
			{
				mode = L"rb+";
			}
			else
			{
				mode = L"wb+";
			}
		}
		else
		{
			mode = L"rb";
		}
		//"r"  NoCeate Read
		//"w"  Create Write Truncate
		//"a"  Create append KeepEOF
		//"r+" NoCeate Read Write
		//"w+" Create Read Write Truncate
		//"a+"  Create append OverwriteEOF
		FILE * file;
#	if defined(UNICODE) && defined(NB_WINDOWS)
		int err = _wfopen_s(&file, textPath.c_str(), mode);
#	else
#		ifdef UNICODE
		stringa t(textPath);
		stringa mode1(mode);
		int err = fopen_s(&file, t.c_str(), mode1.c_str());
#		else
		int err = fopen_s(&file, textPath.c_str(), mode);
#		endif
#	endif
		if(file != 0)
		{
			CrtFile * p = enew CrtFile();
			p->Attach(file, true);
			return p;
		}
		else
		{
			return 0;
		}
	}

	static FileRef _OpenFile(const Path & _path, bool _write)
	{
		E_ASSERT(_path.IsAbsolute());
//		FSRef systemFS = GetSystemFS();
		FileRef ret = _OpenRealFile(_path, _write);
		if(ret)
		{
			//message(L"[nb]   File on real FS: " + _path.GetString());
			return ret;
		}

		if(_write)
		{
			return 0;
		}

		FilePackLoader loader(_path);
		FilePack * fp = loader.Load();
		if(fp)
		{
			ret = fp->OpenFile(loader.relativePath, _write);
			if(ret)
			{
				//message(L"[nb]   File in pack: " + _path.GetString());
				return ret;
			}
		}
		
		//message(L"[nb]   File not found: " + _path.GetString());
		return 0;
	}

	FileRef FS::OpenFile(const Path & _path, bool _write)
	{
		if(!_path.IsValid())
		{
			return 0;
		}

		if(_path.IsAbsolute())
		{
			return _OpenFile(_path, _write);
		}
		else
		{
			return _OpenFile(FS::GetCurrentFolder() | _path, _write);
		}
	}

	static DirectoryRef _OpenFolder(const Path & _path)
	{
		E_ASSERT(_path.IsAbsolute());
		if(IsRealFolder(_path))
		{
			return _OpenRealDir(_path);
		}

		FilePackLoader loader(_path);
		FilePack * fp = loader.Load();
		return fp ? fp->OpenDir(loader.relativePath) : DirectoryRef(0);
	}

	DirectoryRef FS::OpenDir(const Path & _path)
	{
		if(!_path.IsValid())
		{
			return 0;
		}

		if(_path.IsAbsolute())
		{
			return _OpenFolder(_path);
		}
		else
		{
			return _OpenFolder(FS::GetCurrentFolder() | _path);
		}
	}


	bool FS::IsFolder(const Path & _path)
	{
		if(IsRealFolder(_path))
		{
			return true;
		}
		FilePackLoader loader(_path);
		FilePack * fp = loader.Load();
		return fp ? fp->IsFolder(loader.relativePath) : 0;
	}

	bool FS::IsFile(const Path & _path)
	{
		if(IsRealFile(_path))
		{
			return true;
		}
		FilePackLoader loader(_path);
		FilePack * fp = loader.Load();
		return fp ? fp->IsFile(loader.relativePath) : 0;
	}

	bool FS::CreateFolder(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		if(!_path.IsAbsolute())
		{
			return false;
		}

		if(!IsFolder(_path))
		{
			E_TRACE_LINE(string(L"[nb] Creating dir(): _path=") + _path.GetString());
			// 
			if(_path.HasParentFolder() && !FS::CreateFolder(_path.GetParentFolder()))
			{
				return false;
			}

#ifdef NB_WINDOWS
			if(!::CreateDirectory(_path.GetString().c_str(), NULL))
			{
				return false;
			}
#endif

#ifdef NB_LINUX
			if(0 != ::mkdir(_path.GetStringA().c_str(), 0700))
			{
				return false;
			}
#endif
		}

		return true;
	}

	bool FS::Copy(const Path & _src, const Path & _dst)
	{
		// NB_PROFILE_INCLUDE;
		bool ret = true;
		if(_dst.HasParentFolder())
		{
			CreateFolder(_dst.GetParentFolder());
		}
		DirectoryRef it = FS::OpenDir(_src);
		if(it)
		{
			do
			{
				string name = it->GetName();
				if(!FS::Copy(_src | name, _dst | name))
				{
					ret = false;
				}
			}while(it->MoveNext(false));
		}
		else
		{
#ifdef NB_WINDOWS
#	ifdef UNICODE
			ret = ::CopyFileW(_src.GetString().c_str(), _dst.GetString().c_str(), false) ? true : false;
#	else
			ret = ::CopyFileA(_src.GetString().c_str(), _dst.GetString().c_str(), false) ? true : false;
#	endif
#endif

#ifdef NB_LINUX
			stringa cmd = "cp \"" + _src.GetString() + "\" \"" + _dst.GetString() + "\"";
			ret = system(cmd.c_str()) == 0;
#endif
		}
		return ret;
	}

	bool FS::Delete(const Path & _path)
	{
#ifdef NB_WINDOWS
		string t = _path.GetString();
		if(::PathIsDirectory(t.c_str()))
		{
			return ::RemoveDirectory(t.c_str()) ? true : false;
		}
		else
		{
			return ::DeleteFile(t.c_str()) ? true : false;
		}
#endif

#ifdef NB_LINUX
		return ::remove(_path.GetStringA().c_str()) == 0;
#endif
	}

	Path FS::GetCurrentFolder()
	{
#ifdef _WIN32
		Char buf[MAX_PATH];
		if(!::GetCurrentDirectory(MAX_PATH, buf))
		{
			E_ASSERT(0);
		}
		return Path(buf);
#endif
#ifdef NB_LINUX
		char buf[MAX_PATH];
		if(!::getcwd(buf, MAX_PATH))
		{
			E_ASSERT(0);
		}
		return Path(string(buf));
#endif
	}

	bool FS::SetCurrentFolder(const Path & _path)
	{
		// NB_PROFILE_INCLUDE;
		if(!_path.IsAbsolute())
		{
			E_ASSERT(0);
			return false;
		}

#ifdef _WIN32
		if(!::SetCurrentDirectory(_path.GetString().c_str()))
		{
			E_ASSERT(0);
			return false;
		}
#endif

#ifdef NB_LINUX
		if(chdir(_path.GetStringA().c_str()))
		{
			E_ASSERT(0);
			return false;
		}
#endif
		return true;
	}
}
