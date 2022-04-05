
#pragma once
#include <stdio.h>
#include <nbug/core/path.h>
#include <nbug/core/str.h>
#include <nbug/core/obj.h>
#include <nbug/core/ref.h>
#include <nbug/tl/list.h>

namespace e
{
	class File;
	class Directory;
	typedef Ref<File> FileRef;
	typedef Ref<Directory> DirectoryRef;

	struct FS
	{
		static FileRef OpenFile(const Path & _path, bool _write = false);
		static DirectoryRef OpenDir(const Path & _path);
		static bool IsFolder(const Path & _path);
		static bool IsFile(const Path & _path);
		static bool CreateFolder(const Path & _path);
		static bool Copy(const Path & _src, const Path & _dst);
		static bool Delete(const Path & _path);
		static Path GetCurrentFolder();
		static bool SetCurrentFolder(const Path & _path);
		static bool Copy(File * _src, File * _dst, uint64 _size);
		static Path ParseShortcutFile(const Path & _path);
	};

//	struct MemoryBlock
//	{
//		uint32 len;
//		uint8 * data; // use delete[] to free
//	};

	class File : public RefObject
	{
	protected:
		virtual ~File() = 0;
	public:
		virtual bool RandomAccess() const = 0;
		virtual uint64 GetSize() const = 0;
		virtual bool SetSize(uint64 _n) = 0;
		virtual bool Read(void * _buf, size_t _size) = 0;
		virtual bool ReadLine(stringa & _t) = 0;
		virtual size_t ReadSome(void * _buf, size_t _size) = 0;
		virtual bool Write(const void * _buf, size_t _size) = 0;
		virtual bool Unread(unsigned char _byte) = 0;
		virtual bool Write(const stringa & _t) = 0;
		virtual bool WriteLine(const stringa & _t);
		virtual bool Eof() const = 0;
		virtual int64 Tell() const = 0;
		virtual bool Seek(int64 _pos, int _type = SEEK_SET) = 0;
		virtual bool Skip(size_t _size);
		virtual bool Flush();
		virtual Charset ReadBom();
		virtual bool WriteBom(const Charset & _charset);
		//MemoryBlock ReadAll();
	};

	class Directory : public RefObject
	{
	protected:
		//friend class Ref<Directory>;
		virtual ~Directory() = 0;
	public:
//		virtual FSRef GetFS() const = 0;
		virtual bool MoveNext(bool _enterSubDir) = 0;
		virtual bool IsEnd() const = 0;
		virtual string GetName() const = 0;
		virtual const Path & GetRootPath() const = 0;
		virtual Path GetRelativePath() const = 0;
		inline Path GetFullPath() const
		{ return GetRootPath() | GetRelativePath(); }
		virtual bool IsDir() const = 0;
		virtual bool IsHidden() const;
		virtual bool IsInFilePack() const = 0;
		inline FileRef OpenFile(bool _write = false) const
		{ return FS::OpenFile(GetFullPath(), _write); }
		inline DirectoryRef OpenDir() const
		{ return FS::OpenDir(GetFullPath()); }
	};

	//FSRef GetSystemFS();
	bool MatchFileName(const string & _name, const string & _pattern);
	bool CreateFilePack(FileRef _dst, DirectoryRef _src, List<string> & _exclude);
	//FSRef LoadFilePack(FileRef & _src);
#ifdef NB_WINDOWS
	void GetAllDriver(StringArray & _ret);
#endif

	FileRef CreateMemoryFile(void * _buf, size_t _size, bool _delete = false);
	FileRef CreateMemoryFile(size_t _size);

//	stringa LoadWholeFileAsStingA(const Path & _path);
//	stringw LoadWholeFileAsStingW(const Path & _path);
}
