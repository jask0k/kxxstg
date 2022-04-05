
#ifndef E_MISSING_H
#define E_MISSING_H

namespace e
{
#if !defined(_MSC_VER)
#	define strcpy_s(a, b, c) strcpy((a), (c))
#	define wcscpy_s(a, b, c) wcscpy((a), (c))
#	define strcat_s(a, b, c) strcat((a), (c))
#	define wcscat_s(a, b, c) wcscat((a), (c))

//	int sprintf_s(char * _buffer, size_t _sizeOfBuffer, const char * _format, ...);
#	if defined(UNICODE) && defined(NB_WINDOWS)
	int _wfopen_s(FILE ** _pFile, const wchar_t * _filename, const wchar_t * _mode);
#	else
	int fopen_s(FILE ** _pFile, const char * _filename, const char * _mode);
#	endif
#endif
}

#endif
